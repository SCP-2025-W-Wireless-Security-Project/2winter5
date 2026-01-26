'use client';

import { useEffect, useState } from 'react';
import { Sidebar } from '../components/sidebar';
import { Navbar } from '../components/navbar';
import { StatWidget } from '../components/stat-widget';
import { ComplexTable } from '../components/complex-table';
import { Card } from '../components/card';
import {
    LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer,
    BarChart, Bar, CartesianGrid
} from 'recharts';

interface LogEntry {
    id: number;
    timestamp: string;
    level: string;
    message: string;
    original: string;
}

export default function Home() {
    const [logs, setLogs] = useState<LogEntry[]>([]);
    const [stats, setStats] = useState({
        total: 0,
        critical: 0,
        warning: 0
    });

    // Dynamic Chart Data State
    const [trendData, setTrendData] = useState<any[]>([]);
    const [typeData, setTypeData] = useState<any[]>([]);

    // System Status State
    const [systemStatus, setSystemStatus] = useState({ interface: 'Searching...', status: 'CHECKING' });

    const [mounted, setMounted] = useState(false);

    useEffect(() => {
        setMounted(true);
        // Polling logs
        const fetchLogs = async () => {
            try {
                const res = await fetch('/api/logs');
                const data = await res.json();

                // Update System Status
                if (data.systemStatus) {
                    setSystemStatus(data.systemStatus);
                }

                if (data.logs) {
                    const fetchedLogs = data.logs;
                    setLogs(fetchedLogs);

                    // 1. Calculate Stats
                    const critical = fetchedLogs.filter((l: LogEntry) => l.level === 'CRITICAL').length;
                    const warning = fetchedLogs.filter((l: LogEntry) => l.level === 'WARNING').length;
                    setStats({
                        total: fetchedLogs.length,
                        critical,
                        warning
                    });

                    // 2. Process Threat Frequency (Group by Time HH:mm)
                    const timeMap = new Map<string, number>();
                    fetchedLogs.forEach((l: LogEntry) => {
                        const timeParts = l.timestamp.split(' ');
                        if (timeParts.length > 1) {
                            const timeStr = timeParts[1].substring(0, 5); // HH:mm
                            timeMap.set(timeStr, (timeMap.get(timeStr) || 0) + 1);
                        }
                    });

                    // Convert map to array and sort by time. Take last 7 entries for the chart.
                    const sortedTrend = Array.from(timeMap.entries())
                        .map(([time, count]) => ({ time, count }))
                        .sort((a, b) => a.time.localeCompare(b.time))
                        .slice(-7); // Show last 7 time buckets

                    setTrendData(sortedTrend);

                    // 3. Process Attack Types
                    const typeCounts = {
                        'Deauth': 0,
                        'Evil Twin': 0,
                        'Auth Flood': 0,
                        'Probe': 0,
                        'Scanning': 0
                    };

                    fetchedLogs.forEach((l: LogEntry) => {
                        const msg = l.message.toLowerCase();
                        if (msg.includes('deauth')) typeCounts['Deauth']++;
                        else if (msg.includes('evil twin')) typeCounts['Evil Twin']++;
                        else if (msg.includes('auth')) typeCounts['Auth Flood']++; // catch 'failed auth' too
                        else if (msg.includes('probe')) typeCounts['Probe']++;
                        else if (msg.includes('scan')) typeCounts['Scanning']++;
                    });

                    const processedTypeData = Object.entries(typeCounts)
                        .map(([name, val]) => ({ name, val }))
                        .filter(item => item.val > 0); // Only show active types

                    setTypeData(processedTypeData);
                }
            } catch (err) {
                console.error(err);
            }
        };
        fetchLogs();
        const interval = setInterval(fetchLogs, 2000);
        return () => clearInterval(interval);
    }, []);

    if (!mounted) return null;

    return (
        <div className="app-container font-sans text-gray-800">
            {/* Main Wrapper - Transparent to show gaps */}
            <div className="app-content-wrapper bg-transparent shadow-none border-none backdrop-filter-none">

                {/* Sidebar - Transparent on grid */}
                <div className="w-full md:w-64 hidden md:block pl-2">
                    <Sidebar />
                </div>

                <div className="flex-1 flex flex-col relative overflow-hidden glass-panel h-full">

                    {/* Fixed Header Section */}
                    <div className="flex-none px-6 pt-6 md:px-10 md:pt-10 z-10">
                        <div className="flex justify-between items-center pb-6 border-b border-gray-200/5">
                            <div className="flex flex-col gap-1">
                                <div className="flex items-center gap-3">
                                    <h1 className="text-4xl font-black text-transparent bg-clip-text bg-gradient-to-r from-cyan-400 via-blue-500 to-purple-600 tracking-tighter drop-shadow-[0_0_15px_rgba(59,130,246,0.5)]">
                                        2winter5 WIPS Dashboard
                                    </h1>
                                </div>
                                <div className="flex items-center gap-2">
                                    <span className="relative flex h-1.5 w-1.5">
                                        <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-green-400 opacity-75"></span>
                                        <span className="relative inline-flex rounded-full h-1.5 w-1.5 bg-green-500"></span>
                                    </span>
                                    <p className="text-gray-400 text-sm font-medium tracking-wide">
                                        Wireless Intrusion Prevention System
                                    </p>
                                </div>
                            </div>
                            <button className="px-5 py-2.5 bg-gradient-to-r from-blue-600 to-indigo-600 hover:from-blue-500 hover:to-indigo-500 text-white rounded-xl font-bold text-sm shadow-lg shadow-blue-500/20 transition-all hover:scale-105 active:scale-95 border border-white/10">
                                리포트 생성
                            </button>
                        </div>
                    </div>

                    {/* Scrollable Content Section */}
                    {/* Padding top 8 (2rem) to match previous space-y-8 gap */}
                    <div className="flex-1 overflow-y-auto px-6 md:px-10 pb-6 md:pb-10 pt-8 space-y-8">

                        {/* Glass Widgets Grid */}
                        <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
                            <div className="glass-card p-6 flex flex-col justify-between h-32">
                                <div className="flex justify-between items-start">
                                    <h4 className="text-gray-400 font-semibold text-sm uppercase tracking-wider">전체 이벤트</h4>
                                    <span className="p-2 bg-blue-500/20 rounded-lg text-blue-400">
                                        <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M13 10V3L4 14h7v7l9-11h-7z"></path></svg>
                                    </span>
                                </div>
                                <div>
                                    <span className="text-3xl font-bold text-white">{stats.total}</span>
                                    <span className="text-xs text-green-400 font-bold ml-2">↑ 실시간</span>
                                </div>
                            </div>

                            <div className="glass-card p-6 flex flex-col justify-between h-32 relative overflow-hidden group">
                                <div className="absolute right-0 top-0 w-24 h-24 bg-red-500/10 rounded-full blur-2xl -mr-10 -mt-10 transition-all"></div>
                                <div className="flex justify-between items-start relative z-10">
                                    <h4 className="text-gray-400 font-semibold text-sm uppercase tracking-wider">치명적 위협</h4>
                                    <span className="p-2 bg-red-500/20 rounded-lg text-red-500">
                                        <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z"></path></svg>
                                    </span>
                                </div>
                                <div className="relative z-10">
                                    <span className="text-3xl font-bold text-white">{stats.critical}</span>
                                    <span className="text-xs text-red-400 font-bold ml-2">조치 필요</span>
                                </div>
                            </div>

                            <div className="glass-card p-6 flex flex-col justify-between h-32">
                                <div className="flex justify-between items-start">
                                    <h4 className="text-gray-400 font-semibold text-sm uppercase tracking-wider">시스템 상태</h4>
                                    <span className={`p-2 rounded-lg ${systemStatus.status === 'UP' || systemStatus.status === 'UNKNOWN' ? 'bg-green-500/20 text-green-400' : 'bg-red-500/20 text-red-400'}`}>
                                        <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z"></path></svg>
                                    </span>
                                </div>
                                <div>
                                    <span className="text-3xl font-bold text-white">
                                        {systemStatus.status === 'UP' || systemStatus.status === 'UNKNOWN' ? '정상' : 'DOWN'}
                                    </span>
                                    {(systemStatus.status === 'UP' || systemStatus.status === 'UNKNOWN') && (
                                        <span className="text-xs text-gray-400 font-medium ml-2">
                                            {systemStatus.interface} 모니터링 중
                                        </span>
                                    )}
                                </div>
                            </div>
                        </div>

                        {/* Charts Section */}
                        <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
                            <div className="glass-card p-6">
                                <h3 className="text-lg font-bold text-white mb-6">위협 빈도</h3>
                                <div className="w-full h-[250px]">
                                    <ResponsiveContainer width="100%" height="100%">
                                        <LineChart data={trendData.length > 0 ? trendData : [{ time: '00:00', count: 0 }]}>
                                            <CartesianGrid strokeDasharray="3 3" vertical={false} stroke="rgba(255,255,255,0.1)" />
                                            {/* Align ticks with data points using interval='preserveStartEnd' or 0 */}
                                            <XAxis dataKey="time" axisLine={false} tickLine={false} tick={{ fill: '#9CA3AF', fontSize: 12 }} dy={10} interval="preserveStartEnd" />
                                            <Tooltip contentStyle={{ borderRadius: '12px', border: '1px solid rgba(255,255,255,0.1)', background: 'rgba(20, 20, 20, 0.9)', color: '#fff' }} itemStyle={{ color: '#fff' }} />
                                            <Line type="monotone" dataKey="count" stroke="#3B82F6" strokeWidth={3} dot={{ r: 4, fill: '#3B82F6', strokeWidth: 2, stroke: '#fff' }} activeDot={{ r: 6, strokeWidth: 0 }} />
                                        </LineChart>
                                    </ResponsiveContainer>
                                </div>
                            </div>

                            <div className="glass-card p-6">
                                <h3 className="text-lg font-bold text-white mb-6">공격 유형</h3>
                                <div className="w-full h-[250px]">
                                    <ResponsiveContainer width="100%" height="100%">
                                        <BarChart data={typeData.length > 0 ? typeData : [{ name: 'None', val: 0 }]} barSize={40}>
                                            <CartesianGrid strokeDasharray="3 3" vertical={false} stroke="rgba(255,255,255,0.1)" />
                                            {/* Fix alignment by ensuring XAxis matches bars */}
                                            <XAxis dataKey="name" axisLine={false} tickLine={false} tick={{ fill: '#9CA3AF', fontSize: 12 }} dy={10} interval={0} />
                                            <Tooltip cursor={{ fill: 'rgba(255, 255, 255, 0.1)', radius: 8 }} contentStyle={{ borderRadius: '12px', border: '1px solid rgba(255,255,255,0.1)', background: 'rgba(20, 20, 20, 0.9)', color: '#fff' }} itemStyle={{ color: '#fff' }} />
                                            <defs>
                                                <linearGradient id="colorGradient" x1="0" y1="0" x2="0" y2="1">
                                                    <stop offset="0%" stopColor="#6366F1" stopOpacity={0.8} />
                                                    <stop offset="95%" stopColor="#6366F1" stopOpacity={0.3} />
                                                </linearGradient>
                                            </defs>
                                            <Bar dataKey="val" fill="url(#colorGradient)" radius={[8, 8, 0, 0]} />
                                        </BarChart>
                                    </ResponsiveContainer>
                                </div>
                            </div>
                        </div>

                        {/* Recent Logs */}
                        <div className="glass-card p-0 overflow-hidden bg-[#242424]/80">
                            <div className="p-6 border-b border-gray-100/10">
                                <h3 className="text-lg font-bold text-white">실시간 보안 로그</h3>
                            </div>
                            <div className="bg-transparent">
                                <ComplexTable logs={logs} />
                            </div>
                        </div>

                        {/* 푸터 */}
                        <footer className="pt-4 pb-2 border-t border-gray-200/30 flex flex-col md:flex-row justify-between items-center text-xs text-gray-400">
                            <div>
                                <p>&copy; 2026 2WINTER5 WIPS</p>
                            </div>
                            <div className="flex gap-4 mt-2 md:mt-0">
                                <span className="flex items-center gap-1.5">
                                    <span className="relative flex h-2 w-2">
                                        <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-green-400 opacity-75"></span>
                                        <span className="relative inline-flex rounded-full h-2 w-2 bg-green-500"></span>
                                    </span>
                                    시스템 정상 가동 중
                                </span>
                            </div>
                        </footer>

                    </div>
                </div>
            </div>
        </div>
    );
}

