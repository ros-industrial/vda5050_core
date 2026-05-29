# To Run the autoxing client Python Bindings -> Spellbook

Requires spellbook venv (`cd ~/autoxing_spellbook/autoxing-spellbook-cli && uv sync`).
Bridge code lives in `autoxing_bridge/`; this folder keeps only the VDA5050 client entrypoint.

**Multi-node routes:** `on_navigate` returns immediately and drives the robot on a worker thread.

```bash
source install/setup.bash
cd vda5050_core_py/autoxing-l300
python3 example_autoxing_client.py
```

**Terminal 2 — publish demo order and verify state:**

```bash
python3 publish_autoxing_l300_route.py --wait-route
```

Optional spellbook-only test CLIs (run from `autoxing-l300/`):

```bash
python3 -m autoxing_bridge.navigate_node -11.0 1.2 0.0
python3 -m autoxing_bridge.poll_robot
python3 -m autoxing_bridge.wait_for_move -11.0 1.2 0.0
```
