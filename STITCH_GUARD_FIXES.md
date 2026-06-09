# VDA5050 Stitch Guard Fixes

## Date: 2026-03-13

## Overview

This document explains the fixes made to `master.py` to address order stitching issues identified during testing.

---

## Issues Identified

### Issue 1: Order Cleared When It Has Only 1 Node

**Problem:**
When an order was reduced to only 1 node (e.g., after combining updates), the master would clear both `self.executing_order` and `self.order`. This caused subsequent order updates to be treated as brand new orders instead of updates to the existing order.

**Symptoms:**
- "WARNING: AGV State message shows the lastNodeSequenceId X is not the last sequence ID of the stored order (Y), but yet the remaining NodeState array is empty"
- AGVs stuck at intermediate positions
- Order updates rejected by AGV

**Root Cause:**
A 1-node order is a valid VDA5050 order representing "AGV at current position, order complete." It should be treated the same as any completed order - `self.executing_order` cleared, but `self.order` kept for future updates.

**The Fix:**
```python
# Before (problematic):
self.executing_order = None
self.order = None  # ← This broke subsequent updates
self.order_id_mismatch_count = 0

# After (fixed):
self.executing_order = None
# Don't clear self.order - keep it for potential updates
self.order_id_mismatch_count = 0
```

**Location:** `update_state_message()`, around line 370

---

### Issue 2: Stale Updates Not Handled (AGV Already Past Stitch Point)

**Problem:**
The stitch guard had checks for:
- Check 1: Is AGV on correct order?
- Check 2: Has AGV reached the stitch point? (queues if AGV is behind)
- Check 3: Has AGV confirmed the previous update?

But there was no check for when the AGV has **already passed** the stitch point. In this case:
- `first_node_seq < self.state.lastNodeSequenceId`
- The update is "stale" - AGV can't stitch backwards
- Sending it to AGV causes `orderUpdateError: Could not stitch order due to invalid sequence ids`

**Symptoms:**
- `orderUpdateError` with `order.node.sequenceId` < `state.baseSequenceId`
- AGV stuck with error in state
- Master repeatedly trying to send rejected updates

**Root Cause:**
The stitch guard only queued updates when AGV hadn't reached the stitch point yet, but didn't handle the case where AGV had already passed it.

**The Fix:**
Added Check 2a before Check 2b:
```python
# Check 2a: Has AGV already PASSED the stitch point? (stale update)
if first_node_seq < self.state.lastNodeSequenceId:
    # Combine to advance orderUpdateId, but don't send to AGV
    self.combine_order(new_order=order_message, automatic_update=True)
    return False  # Don't send - AGV already past this point

# Check 2b: Has AGV reached the stitch point yet?
if first_node_seq > self.state.lastNodeSequenceId:
    # Queue - wait for AGV to catch up
    self.pending_order_updates.append(order_message)
    return False
```

**Key Design Decision:**
Instead of just discarding stale updates, we **combine them** into the master's order. This advances the `orderUpdateId` so that future updates can still be processed. If we simply discarded stale updates, the system could get stuck waiting for an update that was never processed.

**Location:**
- `update_order_message()`, around line 429
- `process_pending_order_updates()`, around line 520

---

## Summary of Changes

| File | Function | Change |
|------|----------|--------|
| `master.py` | `update_state_message()` | Don't clear `self.order` when order has 1 node |
| `master.py` | `update_order_message()` | Add Check 2a for stale updates |
| `master.py` | `process_pending_order_updates()` | Add Check 2a for stale queued updates |

---

## Testing

After applying these fixes, verify:

1. **1-node order handling:**
   - Orders with 1 node should show "completed at current position" (yellow)
   - Subsequent updates should be combined, not treated as new orders

2. **Stale update handling:**
   - When AGV is past stitch point, should show "STALE UPDATE" (yellow)
   - Update should be combined but NOT sent to AGV
   - Future updates should still work

3. **No regressions:**
   - Normal order flow still works
   - Stitch guard queueing still works (Check 2b)
   - AGV confirmation check still works (Check 3)
