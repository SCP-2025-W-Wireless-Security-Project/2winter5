import Image from 'next/image';
import { LayoutDashboard, AlertTriangle, Settings } from 'lucide-react';

export function Sidebar() {
    return (
        <div className="h-full py-6 px-4 flex flex-col gap-2 bg-transparent">
            <div className="px-4 mb-8 flex items-center gap-4">
                <div className="relative w-11 h-11">
                    <Image src="/Logo.png" alt="2winter5 Logo" fill className="object-contain" />
                </div>
                <span className="font-bold text-2xl text-white">2winter5</span>
            </div>

            <div className="space-y-1">
                <NavItem icon={<LayoutDashboard size={20} />} label="대시보드" active />
                <NavItem icon={<AlertTriangle size={20} />} label="알림" />
            </div>
        </div>
    );
}

function NavItem({ icon, label, active = false }: { icon: React.ReactNode, label: string, active?: boolean }) {
    return (
        <div className={`flex items-center gap-3 px-4 py-3 rounded-lg cursor-pointer transition-all ${active ? 'bg-white/10 text-white font-bold' : 'text-gray-400 hover:bg-white/5 hover:text-gray-200'}`}>
            {icon}
            <span className="text-[15px]">{label}</span>
        </div>
    );
}
