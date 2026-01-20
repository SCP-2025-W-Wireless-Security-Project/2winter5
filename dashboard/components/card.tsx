import { clsx, type ClassValue } from "clsx";
import { twMerge } from "tailwind-merge";

export function cn(...inputs: ClassValue[]) {
    return twMerge(clsx(inputs));
}

export function Card({ children, className, extra }: { children: React.ReactNode; className?: string, extra?: string }) {
    return (
        <div
            className={cn(
                "card",
                className,
                extra
            )}
        >
            {children}
        </div>
    );
}
