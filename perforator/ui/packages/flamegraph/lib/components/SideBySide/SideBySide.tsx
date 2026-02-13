import * as React from 'react';

import { Divider } from '@gravity-ui/uikit';

import { parseStacks } from '../../query-utils';
import { search as outerSearch } from '../../search';
import { calculateTopForTable } from '../../top';
import { cn } from '../../utils/cn';
import { Flamegraph, type FlamegraphProps } from '../Flamegraph/Flamegraph';
import { TopTable, type TopTableProps } from '../TopTable/TopTable';

import './SideBySide.css';


export type SideBySideProps = FlamegraphProps & Pick<TopTableProps, 'navigate'>

const b = cn('visualisation_sbs');

export function SideBySide(props: SideBySideProps) {
    const { profileData, getState } = props;
    const frameDepth = parseInt(getState('frameDepth', '0'));
    const framePos = parseInt(getState('framePos', '0'));
    const omitted = getState('omittedIndexes', '');
    const keepOnlyFound = getState('keepOnlyFound') === 'true';
    const search = getState('flamegraphQuery');
    const exactMatch = getState('exactMatch');
    const excludeSearch = getState('flamegraphExclude');

    const searchFn = React.useCallback((query: RegExp | string, omitQuery?: RegExp | string) => {
        if (!profileData.rows) {
            return [];
        }
        function readString(index: number) {
            return profileData?.stringTable[index] ?? '';
        }

        return outerSearch(readString, (str) => str, false, profileData.rows, query, omitQuery);
    }, [profileData.rows, profileData?.stringTable]);
    const topData = React.useMemo(() => {
        let keepCoords = null;
        if (keepOnlyFound && search) {

            keepCoords = searchFn(
                exactMatch === 'true' ? decodeURIComponent(search) : RegExp(decodeURIComponent(search)),
                exactMatch === 'true' ? decodeURIComponent(excludeSearch) : RegExp(decodeURIComponent(excludeSearch)),
            );
        }
        return profileData ? calculateTopForTable(profileData.rows, profileData.stringTable.length, { rootCoords: [frameDepth, framePos] as [number, number], omitted: parseStacks(omitted), keepCoords }) : null;
    }, [keepOnlyFound, search, profileData, frameDepth, framePos, omitted, searchFn, exactMatch, excludeSearch]);
    return <div className={b()}>
        <TopTable
            lines={100}
            disableAutoTabSwitch
            className={b('top-table')}
            {...props}
            topData={topData!}
        />
        <Divider orientation={'vertical'} />
        <Flamegraph
            useSelfAsScrollParent
            className={b('flamegraph')}
            {...props}
        />
    </div>;
}
