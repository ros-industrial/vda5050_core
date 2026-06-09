# Changelog

## [2024-03-17] Per-Robot Command Queue System

### Problem Statement

A race condition existed in the ORDER_UPDATE_ID handling when ADG (Autonomous Driving Graph) sent rapid command updates via FIWARE/Scorpio. When `commandUpdateId=0` and `commandUpdateId=1` arrived in quick succession:

1. `updateId=0` arrives, robot starts processing
2. `updateId=1` arrives before robot state reflects the new order
3. ORDER FIX incorrectly resets `updateId=1` to `updateId=0` (because robot state still shows old order)
4. Robot rejects the second order as duplicate (`updateId=0` already received)
5. Robot gets stuck - never receives the stitching waypoints from `updateId=1`

### Root Cause

The ORDER FIX in `master.py` relied on comparing `robot_current_order != incoming_order` to decide whether to reset `orderUpdateId`. This check was fundamentally flawed because:

- Robot state updates are asynchronous
- During rapid updates, the robot state may not yet reflect the newly accepted order
- This caused legitimate stitching updates (`updateId=1`) to be incorrectly reset to `updateId=0`

### Solution: Per-Robot Command Queue

Implemented a per-robot command queue in `vda5050_fiware.py` that:

1. **Queues all incoming CommandMessage updates** per robot
2. **Tracks updateIds independently** per (robot_id, orderId) - not relying on robot state
3. **Processes updates in correct order** (send updateId=2 before updateId=3)
4. **Handles missing updates gracefully**:
   - If `updateId=0` is lost and `updateId=1` arrives first, reset it to `updateId=0`
   - Wait briefly (2 seconds) for missing updates before skipping gaps
5. **Discards stale updates** that arrive after a higher updateId was already sent

### Files Changed

#### `vda5050_fiware/vda5050_fiware.py`

Added command queue system with the following components:

| Component | Description |
|-----------|-------------|
| `command_queues` | Dict mapping robot_id to list of pending OrderMessages |
| `last_sent_updateId` | Dict tracking last sent updateId per (robot_id, orderId) |
| `queue_lock` | Threading lock for thread-safe queue access |
| `queue_timers` | Timer threads for handling missing update gaps |
| `missing_update_timeout` | Configurable timeout (default: 2.0 seconds) |

New methods:

| Method | Description |
|--------|-------------|
| `_get_robot_id()` | Extract robot ID from order message |
| `_process_queue()` | Main queue processing logic |
| `_send_order()` | Send order and update tracking state |
| `_on_timeout()` | Handle timeout when waiting for missing updates |

#### `vda5050_fiware/master.py`

- **Removed**: ORDER FIX code block (lines 416-440) that incorrectly reset orderUpdateId
- **Kept**: STITCH GUARD logic for runtime state checks during order execution

### Queue Processing Logic

```
on_command_message(order):
    queue[robot_id].append(order)
    process_queue(robot_id)

process_queue(robot_id):
    sort queue by (orderId, orderUpdateId)
    order = queue[0]
    last_sent = tracking[robot_id][order.orderId]

    if order.updateId <= last_sent:
        # Stale update - discard
        queue.pop(0)
        continue processing

    expected = last_sent + 1

    if order.updateId == expected OR (last_sent == -1 AND updateId == 0):
        # Perfect - send it
        send_order(order)

    elif last_sent == -1 AND updateId > 0:
        # updateId=0 was lost, reset this one to 0
        order.updateId = 0
        send_order(order)

    else:
        # Gap detected - wait for missing update
        start_timer(timeout=2s, callback=skip_gap_and_send)
```

### Scenario Handling

| Scenario | Queue Behavior |
|----------|----------------|
| Normal: 0, 1, 2 arrive in order | Send immediately as each arrives |
| Out-of-order: 1 arrives before 0 | Queue 1, wait 2s for 0, then reset 1→0 and send |
| Lost initial: 0 lost, only 1 arrives | Reset 1→0 and send immediately |
| Gap: 0, 1, 3 arrive (2 missing) | Send 0, send 1, wait 2s for 2, skip gap, send 3 |
| Stale: 2 arrives after 3 was sent | Discard 2 |
| Rapid updates: 0, 1 arrive quickly | Queue ensures 0 sent first, then 1 |

### Log Messages

Queue system logs are prefixed with `[QUEUE]`:

- `[QUEUE] Received command for MiR_0001: orderId=..., updateId=...`
- `[QUEUE] Processing MiR_0001: orderId=..., updateId=..., lastSent=...`
- `[QUEUE] Sending order to MiR_0001: orderId=..., updateId=...`
- `[QUEUE] Discarding stale update for MiR_0001: updateId=X <= lastSent=Y`
- `[QUEUE] updateId=0 missing for MiR_0001, resetting updateId=1 to 0`
- `[QUEUE] Gap detected for MiR_0001: have updateId=3, need updateId=2. Waiting 2.0s...`
- `[QUEUE] Timeout waiting for updateId=2. Skipping gap, sending updateId=3`

### Testing

To test the fix:

1. Start vda5050_fiware with simulated robots
2. Send rapid CommandMessage updates via Scorpio with updateId=0 and updateId=1
3. Verify both updates are processed correctly (check `[QUEUE]` logs)
4. Test edge cases: out-of-order arrivals, missing updates, stale updates

Test script available: `modules/src/vda5050_client/scripts/test_stitching.py`

### Migration Notes

- No configuration changes required
- Queue timeout is hardcoded to 2.0 seconds (can be made configurable if needed)
- Existing STITCH GUARD logic in master.py is preserved for runtime state checks
