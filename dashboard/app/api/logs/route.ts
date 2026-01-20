import { NextResponse } from 'next/server';
import fs from 'fs';
import path from 'path';

export async function GET() {
    try {
        // Log file path: go up from dashboard/ to wips_project/attack.log
        const logPath = path.join(process.cwd(), '..', 'attack.log');

        if (!fs.existsSync(logPath)) {
            return NextResponse.json({ logs: [] });
        }

        const fileContent = fs.readFileSync(logPath, 'utf-8');
        const logs = fileContent
            .split('\n')
            .filter(line => line.trim() !== '')
            .map((line, index) => {
                // Simple regex to parse log: [Date Time] [Level] Message
                const match = line.match(/^\[(.*?)\] \[(.*?)\] (.*)$/);
                if (match) {
                    return {
                        id: index,
                        timestamp: match[1],
                        level: match[2],
                        message: match[3],
                        original: line
                    };
                }
                return {
                    id: index,
                    timestamp: '-',
                    level: 'UNKNOWN',
                    message: line,
                    original: line
                };
            })
            .reverse(); // Show newest first

        // 4. Check System Status (Interface wlan0mon or wlan0)
        let interfaceName = 'Unknown';
        let interfaceStatus = 'DOWN';

        try {
            const netDir = '/sys/class/net';
            if (fs.existsSync(netDir)) {
                const interfaces = fs.readdirSync(netDir);
                // Prioritize wlan0mon, then wlan0, then any wlan*
                const targetInterface = interfaces.find(iface => iface === 'wlan0mon') ||
                    interfaces.find(iface => iface === 'wlan0') ||
                    interfaces.find(iface => iface.startsWith('wlan'));

                if (targetInterface) {
                    interfaceName = targetInterface;
                    const operstatePath = path.join(netDir, targetInterface, 'operstate');
                    if (fs.existsSync(operstatePath)) {
                        interfaceStatus = fs.readFileSync(operstatePath, 'utf-8').trim().toUpperCase();
                    }
                }
            }
        } catch (e) {
            console.error('Error checking network status:', e);
        }

        return NextResponse.json({ logs, systemStatus: { interface: interfaceName, status: interfaceStatus } });
    } catch (error) {
        console.error('Error reading log file:', error);
        return NextResponse.json({ error: 'Failed to read logs' }, { status: 500 });
    }
}
