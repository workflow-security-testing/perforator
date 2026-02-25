import type { DenselyPackedCoordinates } from './densely-packed';
import { pushDenseCoord } from './densely-packed';
import type { ProfileData } from './models/Profile';
import type { ReadString, StringModifier } from './node-title';
import { getNodeTitleFull } from './node-title';


// TODO replace with RegExp.escape once ES2025 is adopted by typescript and babel
export function escapeRegex(str: string) {
    return str.replace(/[/\-\\^$*+?.()|[\]{}]/g, '\\$&');
}

export function makeTestFn(query: RegExp | undefined, caseInsensitive = false) {
    if (typeof query === 'object' && query instanceof RegExp) {
        if (caseInsensitive && !query.flags.includes('i')) {
            return (str: string) => new RegExp(query.source, query.flags + 'i').test(str);
        }
        return (str: string) => query.test(str);
    }
    return () => false;
}

export function search(readString: ReadString, shorten: StringModifier, shouldShorten: boolean, rows: ProfileData['rows'], query: RegExp, excludeQuery?: RegExp, caseInsensitive = false): DenselyPackedCoordinates {
    const res: DenselyPackedCoordinates = [];
    const test = makeTestFn(query, caseInsensitive);
    const excludeTest = makeTestFn(excludeQuery, caseInsensitive);

    const getNodeTitle = getNodeTitleFull.bind(null, readString, shorten, shouldShorten);

    for (let h = 0; h < rows.length; h++) {
        for (let i = 0; i < rows[h].length; i++) {
            const node = rows[h][i];
            const name = getNodeTitle(node);
            const matched = test(name);
            const excluded = excludeTest(name);
            if (matched && !excluded) {
                pushDenseCoord(res, h, i);
            }
        }
    }

    return res;
}
