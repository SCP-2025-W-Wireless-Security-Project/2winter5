import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  /* config options here */
  experimental: {
    serverActions: {
      allowedOrigins: ["192.168.35.129:3000", "localhost:3000"],
    },
  },
  // Suppress specific warnings if needed, but the main fix is the allowedOrigins
};

export default nextConfig;
