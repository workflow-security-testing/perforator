import * as React from 'react';
import { useState } from 'react';


const DEFAULT_HEIGHT = 20;

export function useLevelHeight(container: React.RefObject<HTMLElement>) {
    const [height, setHeight] = useState<number | null>(DEFAULT_HEIGHT);

    React.useLayoutEffect(() => {
        const getCssVariable = (variable: string) => {
            return getComputedStyle(container.current!).getPropertyValue(variable);
        };
        setHeight(parseInt(getCssVariable('--flamegraph-level-height')!));
    }, [container]);

    return height;
}
