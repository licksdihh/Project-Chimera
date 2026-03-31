import cv2
import numpy as np
import serial
import json
import asyncio
import websockets

# --- Serial from MCU ---
ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)

# --- Stereo Cameras ---
capL = cv2.VideoCapture(0)
capR = cv2.VideoCapture(1)

# --- Stereo Matcher ---
stereo = cv2.StereoBM_create(numDisparities=64, blockSize=15)

# --- WebSocket Clients ---
clients = set()

async def register(ws):
    clients.add(ws)

async def unregister(ws):
    clients.remove(ws)

async def broadcast(data):
    if clients:
        await asyncio.wait([ws.send(data) for ws in clients])

# --- Hazard Detection ---
def detect_hazard(disparity):
    mean_depth = np.mean(disparity)
    if mean_depth < 20:
        return "PIT"
    elif mean_depth > 150:
        return "OBSTACLE"
    return "CLEAR"

# --- Main Loop ---
async def vision_loop():
    while True:
        retL, frameL = capL.read()
        retR, frameR = capR.read()

        if not retL or not retR:
            continue

        grayL = cv2.cvtColor(frameL, cv2.COLOR_BGR2GRAY)
        grayR = cv2.cvtColor(frameR, cv2.COLOR_BGR2GRAY)

        disparity = stereo.compute(grayL, grayR)
        disparity = cv2.normalize(disparity, None, 0, 255, cv2.NORM_MINMAX)

        hazard = detect_hazard(disparity)

        # --- Read MCU Data ---
        try:
            line = ser.readline().decode().strip()
            sensor_data = json.loads(line)
        except:
            sensor_data = {}

        payload = {
            "hazard": hazard,
            "mq135": sensor_data.get("mq135", 0),
            "mq2": sensor_data.get("mq2", 0),
            "distance": sensor_data.get("distance", 0),
            "alert": sensor_data.get("alert", False),
            "depth_map": disparity.tolist()[::20]  # downsample
        }

        await broadcast(json.dumps(payload))

        await asyncio.sleep(0.1)

# --- WebSocket Server ---
async def handler(ws):
    await register(ws)
    try:
        while True:
            await asyncio.sleep(1)
    finally:
        await unregister(ws)

async def main():
    server = await websockets.serve(handler, "0.0.0.0", 8765)
    await asyncio.gather(server.wait_closed(), vision_loop())

if __name__ == "__main__":
    asyncio.run(main())
