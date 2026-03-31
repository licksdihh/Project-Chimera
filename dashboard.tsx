"use client";

import { useEffect, useState } from "react";

export default function Dashboard() {
  const [data, setData] = useState<any>({});

  useEffect(() => {
    const ws = new WebSocket("ws://localhost:8765");

    ws.onmessage = (event) => {
      setData(JSON.parse(event.data));
    };

    return () => ws.close();
  }, []);

  return (
    <div className="min-h-screen bg-black text-white p-6">
      <h1 className="text-3xl font-bold text-cyan-400 mb-6">
        Project Chimera Dashboard
      </h1>

      {/* System Health */}
      <div className="mb-6 p-4 rounded-2xl bg-white/10 backdrop-blur-lg border border-cyan-400">
        <p>System Status: {data.alert ? "⚠ ALERT" : "✅ NORMAL"}</p>
        <p>Hazard: {data.hazard}</p>
      </div>

      {/* Gauges */}
      <div className="grid grid-cols-2 gap-4 mb-6">
        <Gauge label="MQ-135" value={data.mq135} />
        <Gauge label="MQ-2" value={data.mq2} />
      </div>

      {/* Depth Map */}
      <div className="p-4 rounded-2xl bg-white/10 backdrop-blur-lg border border-pink-500">
        <h2 className="text-pink-400 mb-2">Depth Map</h2>
        <canvas id="depthCanvas" width={300} height={200}></canvas>
      </div>
    </div>
  );
}

function Gauge({ label, value }: any) {
  return (
    <div className="p-4 rounded-2xl bg-white/10 backdrop-blur-lg border border-cyan-400">
      <p className="text-cyan-300">{label}</p>
      <p className="text-2xl">{value}</p>
    </div>
  );
}

// --- Depth Map Renderer ---
if (typeof window !== "undefined") {
  const canvas = document.getElementById("depthCanvas") as HTMLCanvasElement;
  if (canvas) {
    const ctx = canvas.getContext("2d");

    setInterval(() => {
      // Placeholder random rendering (replace with actual data binding)
      for (let y = 0; y < 200; y++) {
        for (let x = 0; x < 300; x++) {
          const val = Math.random() * 255;
          ctx!.fillStyle = `rgb(${val},0,${255 - val})`;
          ctx!.fillRect(x, y, 1, 1);
        }
      }
    }, 500);
  }
}
