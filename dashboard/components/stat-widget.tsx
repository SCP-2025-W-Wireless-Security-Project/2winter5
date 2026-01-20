import { Card } from "./card";

export function StatWidget({
    title,
    value,
    subtitle,
    className
}: {
    title: string;
    value: string;
    subtitle?: string;
    className?: string;
}) {
    return (
        <Card className={className}>
            <div>
                <h4 className="text-sm font-medium text-[#A3AED0] mb-1">{title}</h4>
                <div className="flex items-baseline gap-2">
                    <span className="text-3xl font-bold text-[#2B3674]">{value}</span>
                    {subtitle && (
                        <span className="text-sm text-[#05CD99] font-medium">
                            {subtitle}
                        </span>
                    )}
                </div>
            </div>
        </Card>
    );
}
