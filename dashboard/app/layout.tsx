import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
    title: "2winter5 Dashboard",
    description: "Wireless Intrusion Prevention System",
    icons: {
        icon: '/Logo.png',
    },
};

export default function RootLayout({
    children,
}: Readonly<{
    children: React.ReactNode;
}>) {
    return (
        <html lang="en">
            <body>{children}</body>
        </html>
    );
}
