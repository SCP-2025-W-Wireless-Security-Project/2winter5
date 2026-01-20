import { Card } from './card';

interface LogEntry {
    id: number;
    timestamp: string;
    level: string;
    message: string;
    original: string;
}

export function ComplexTable({ logs }: { logs: LogEntry[] }) {
    return (
        <div className="w-full">
            <div className="flex justify-between items-center mb-6 pl-6">
                <h2 className="text-xl font-bold text-white tracking-tight">상세 로그</h2>
            </div>

            <div className="overflow-x-auto w-full">
                <table className="w-full">
                    <thead>
                        <tr className="border-b border-gray-700">
                            <th className="pb-4 pl-6 pr-4 text-center text-xs font-bold text-gray-400 uppercase tracking-wide w-20">ID</th>
                            <th className="pb-4 pr-4 text-center text-xs font-bold text-gray-400 uppercase tracking-wide w-32">상태</th>
                            <th className="pb-4 pr-4 text-center text-xs font-bold text-gray-400 uppercase tracking-wide w-48">일시</th>
                            <th className="pb-4 pr-4 text-start text-xs font-bold text-gray-400 uppercase tracking-wide">이벤트 상세</th>
                        </tr>
                    </thead>
                    <tbody>
                        {logs.length === 0 ? (
                            <tr><td colSpan={4} className="text-center py-8 text-gray-500">No data found.</td></tr>
                        ) : (
                            logs.map((log) => (
                                <tr key={log.id} className="group hover:bg-white/5 transition-colors border-b border-gray-800 last:border-none">
                                    <td className="py-4 pl-6 pr-4 text-sm font-bold text-gray-200 text-center">{log.id}</td>
                                    <td className="py-4 pr-4 text-center">
                                        <div className="flex items-center justify-center">
                                            <span className={`text-sm font-bold px-3 py-1 rounded-full ${log.level === 'CRITICAL' ? 'bg-red-500/20 text-red-400' :
                                                log.level === 'WARNING' ? 'bg-amber-500/20 text-amber-400' :
                                                    'bg-green-500/20 text-green-400'
                                                }`}>
                                                {log.level}
                                            </span>
                                        </div>
                                    </td>
                                    <td className="py-4 pr-4 text-sm font-bold text-gray-200 text-center">{log.timestamp}</td>
                                    <td className="py-4 pr-4 text-sm font-bold text-gray-300">{log.message}</td>
                                </tr>
                            ))
                        )}
                    </tbody>
                </table>
            </div>
        </div>
    );
}
