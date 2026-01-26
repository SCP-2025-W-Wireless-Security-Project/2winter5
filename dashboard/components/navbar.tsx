import { Search, Bell, Info, Moon } from 'lucide-react';

export function Navbar({ title }: { title: string }) {
    return (
        <nav className="sticky top-4 z-40 bg-white/10 backdrop-blur-xl rounded-xl p-3 flex justify-between items-center mb-6 mx-2">
            <div>
                <p className="text-sm font-medium text-[#707EAE]">Pages / {title}</p>
                <h1 className="text-3xl font-bold text-[#2B3674]">{title}</h1>
            </div>

            <div className="bg-white p-2.5 rounded-full flex items-center shadow-[14px_17px_40px_4px_rgba(112,144,176,0.18)] gap-3">
                <div className="flex items-center bg-[#F4F7FE] rounded-full px-4 py-2">
                    <Search className="h-4 w-4 text-[#2B3674] mr-2" />
                    <input
                        type="text"
                        placeholder="Search..."
                        className="bg-transparent border-none outline-none text-sm text-[#2B3674] placeholder-[#8F9BBA] w-32"
                    />
                </div>

                <button className="text-[#8F9BBA] hover:text-[#2B3674]">
                    <Bell className="h-5 w-5" />
                </button>
                <button className="text-[#8F9BBA] hover:text-[#2B3674]">
                    <Info className="h-5 w-5" />
                </button>
                <button className="text-[#8F9BBA] hover:text-[#2B3674]">
                    <Moon className="h-5 w-5" />
                </button>
                <div className="h-10 w-10 rounded-full bg-gradient-to-b from-[#4318FF] to-[#3965FF] flex items-center justify-center text-white font-bold">
                    AD
                </div>
            </div>
        </nav>
    );
}
