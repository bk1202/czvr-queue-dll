# CZVR Request Queue - Euroscope Plugin (v0.1 scaffold)

## What this does right now

- Watches every flight plan once per second (`OnTimer`).
- When a flight plan's ground state becomes `TAXI` (EuroScope's own "taxiing out"
  state), it's automatically appended to an internal priority queue.
- A TAG item ("REQ Queue Status") shows `#<position> <mm:ss waited>` on any
  aircraft currently in the queue, e.g. `#2 04:12`.
- A TAG function ("REQ Queue Menu") opens a right-click-style popup with
  Move to Top / Move Up / Move Down / Remove from Queue, so a controller can
  override the FIFO order manually.
- Aircraft are dropped from the queue automatically once they leave TAXI
  (progress to DEPA, get pushed back to PUSH, disconnect, etc).


## Setting it up in EuroScope 

1. Settings menu -> Plug-ins -> Load
2. Select `CZVR_ReqQueue.dll`
3. In your DEP/GND tag layout editor, add a new column, set its type to
   "REQ Queue Status"
4. Bind a mouse button on that same column to the "REQ Queue Menu" function

