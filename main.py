from fastapi import FastAPI, HTTPException, Query, Path
from pydantic import BaseModel
from fastapi.middleware.cors import CORSMiddleware
import sqlite3

conn = sqlite3.connect("devices.db")

app = FastAPI()

class Device(BaseModel):
    id: int
    device_name: str
    value: str

origins = ["*"]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.post("/devices", response_model=Device, status_code=201)
async def create_device(device: Device):
    try:
        c = conn.cursor()
        c.execute('INSERT INTO devices (id, device_name, value) VALUES (?, ?, ?);', (device.id, device.device_name, device.value))
        conn.commit()
        return device
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/devices")
async def get_devices():
    try:
        c = conn.cursor()
        c.execute('SELECT * FROM devices;')
        response = [{"id": row[0], "device_name": row[1], "value": row[2]} for row in c.fetchall()]
        return response
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/devices/{id}")
async def get_device_status(id: int = Path(..., title="Device ID")):
    try:
        c = conn.cursor()
        c.execute('SELECT value FROM devices WHERE id = ?', (id,))
        value = c.fetchone()
        if value is not None:
            return {"value": value[0]}
        else:
            raise HTTPException(status_code=404, detail="Device not found")
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.patch("/devices/{id}/{new_value}")
async def update_device_status(id: int, new_value: str):
    try:
        c = conn.cursor()
        c.execute('UPDATE devices SET value = ? WHERE id = ?', (new_value, id))
        conn.commit()
        return {"mensaje": "Estado/Valor Actualizado"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))