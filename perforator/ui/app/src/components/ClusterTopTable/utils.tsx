import dayjs from '@gravity-ui/date-utils/build/dayjs';

import type { ClusterTopCount, ClusterTopEntry, ClusterTopGeneration } from 'src/generated/perforator/proto/perforator/perforator';


export interface ClusterTopRow {
    Name: string;
    Count: {
        Self: number;
        Cumulative: number;
        SelfPct: string;
        CumulativePct: string;
    };
    type: 'function' | 'service';
    services?: ClusterTopRow[];
    isLoadingServices?: boolean;
    error?: string;
    // present only for services
    parentFunction?: string;
}

function mapCount(count: ClusterTopCount | undefined, timeInterval: number) {
    return {
        Self: Math.round((count?.Self ?? 0) / timeInterval),
        Cumulative: Math.round((count?.Cumulative ?? 0) / timeInterval),
        SelfPct: (count?.SelfPct ?? 0).toFixed(2) + '%',
        CumulativePct: (count?.CumulativePct ?? 0).toFixed(2) + '%',
    };
}

export function convertToFunctionRow(entry: ClusterTopEntry, timeInterval: number): ClusterTopRow {
    return {
        Name: entry.Name,
        Count: mapCount(entry.Count, timeInterval),
        type: 'function',
        services: undefined,
        isLoadingServices: false,
    };
}

export function convertToServiceRow(parentName: string, entry: ClusterTopEntry, timeInterval: number): ClusterTopRow {
    return {
        Name: entry.Name,
        Count: mapCount(entry.Count, timeInterval),
        type: 'service',
        parentFunction: parentName,
    };
}


export function countHoursInterval(generation: ClusterTopGeneration) {
    const timeInterval = dayjs(generation.To).diff(dayjs(generation.From), 'hours');
    return timeInterval;
}
