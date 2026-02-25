import React from 'react';

import { escapeRegex } from '../search';


export function useSearchPattern(search: string, excludeSearch: string, exactMatch: boolean, caseInsensitive: boolean) {
    return React.useMemo(() => {
        const createSearchPattern = (pattern: string) => {
            if (!pattern) {
                return null;
            }
            const flags = caseInsensitive ? 'i' : undefined;
            if (exactMatch) {
                return new RegExp(escapeRegex(decodeURIComponent(pattern)), flags);
            }
            return new RegExp(decodeURIComponent(pattern), flags);
        };

        return {
            searchPattern: createSearchPattern(search),
            excludeSearchPattern: createSearchPattern(excludeSearch),
        };
    }, [exactMatch, excludeSearch, caseInsensitive, search]);
}
