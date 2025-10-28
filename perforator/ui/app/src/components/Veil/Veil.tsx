import React from 'react';

import './Veil.css';


export interface HighlightVeilData {
    x: number;
    y: number;
    width: number;
    height: number;
    topOffset: number;
}

export type HighlightVeilProps = {
    getHighlightVeilData: () => HighlightVeilData | undefined;
};

export const HighlightVeil: React.FC<HighlightVeilProps> = ({ getHighlightVeilData }) => {
    const divRef = React.useRef<HTMLDivElement | null>(null);
    React.useEffect(() => {
        const redrawVeil = () => setTimeout(() => {
            const highlightVeilData = getHighlightVeilData();
            if (highlightVeilData && divRef.current) {
                const { x, y, width, height, topOffset } = highlightVeilData;
                const clipPathString = `polygon(evenodd, 0 ${topOffset}px, 100% ${topOffset}px, 100% 100%, 0 100%, 0 0, ${x}px ${y}px, ${x + width}px ${y}px, ${x + width}px ${y + height}px, ${x}px ${y + height}px, ${x}px ${y}px)`;

                divRef.current.style.clipPath = clipPathString;
            }
        }, 0);
        redrawVeil();
        window.addEventListener('resize', redrawVeil);
        return () => {
            window.removeEventListener('resize', redrawVeil);
        };
    });
    return (
        <div
            ref={divRef}
            className="highlight__veil"
        >
        </div>
    );
};
