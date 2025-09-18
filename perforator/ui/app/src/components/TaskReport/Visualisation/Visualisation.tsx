import React from 'react';

import { useNavigate } from 'react-router-dom';

import type { FlamegraphProps, QueryKeys, TopTableProps } from '@perforator/flamegraph';
import { calculateTopForTable as calculateTopForTableOriginal, Flamegraph, SideBySide, TopTable } from '@perforator/flamegraph';

import { Loader } from '@gravity-ui/uikit';
import { Tabs } from '@gravity-ui/uikit/legacy';

import { Beta } from 'src/components/Beta/Beta';
import { useFullscreen } from 'src/components/Fullscreen/FullscreenContext';
import { Link } from 'src/components/Link/Link';
import { uiFactory } from 'src/factory';
import { withMeasureTime } from 'src/utils/logging';
import { measureBrowserMemory } from 'src/utils/performance';
import { useTypedQuery } from 'src/utils/query';
import { createSuccessToast } from 'src/utils/toaster';

import type { Tab } from '../TaskFlamegraph/TaskFlamegraph';

import './Visualisation.css';


const calculateTopForTable = withMeasureTime(calculateTopForTableOriginal, 'calculateTopForTable', (ms) => uiFactory().rum()?.sendDelta?.('calculateTopForTable', ms));


type FlamegraphSizes = 'xs' | 's' | 'm' | 'l' | 'xl' | 'xxl';

const enum SizeThresholds {
    XS = 10_000,
    S = 50_000,
    M = 100_000,
    L = 500_000,
    XL = 1_000_000,
}

function getFlamegraphSize(n: number): FlamegraphSizes {
    if (n < SizeThresholds.XS) {
        return 'xs';
    }
    else if (n < SizeThresholds.S) {
        return 's';
    }
    else if (n < SizeThresholds.M) {
        return 'm';
    }
    else if (n < SizeThresholds.L) {
        return 'l';
    }
    else if (n < SizeThresholds.XL) {
        return 'xl';
    }

    return 'xxl';
}

export interface VisualisationProps extends Pick<FlamegraphProps,
'profileData'
 | 'isDiff'
 | 'theme'
 | 'userSettings'
 > {
    loading: boolean;
}

const RUM_BASE_NAME = 'flamegraph-render';


export const Visualisation: React.FC<VisualisationProps> = ({ profileData, ...props }) => {
    const navigate = useNavigate();
    const [getQuery, setQuery] = useTypedQuery<'tab' | QueryKeys>();
    const tab: Tab = getQuery('tab', 'flame') as Tab;
    const isTopTab = tab === 'top' || tab === 'sbs';
    const [isFirstTopRender, setIsFirstTopRender] = React.useState(isTopTab);
    React.useEffect(() => {
        setIsFirstTopRender(isFirstTopRender || isTopTab);
    }, [isFirstTopRender, isTopTab]);
    const { setEnabled } = useFullscreen();
    React.useEffect(() => {
        if (tab === 'sbs') {
            setEnabled(true);
        }
        else {
            setEnabled(false);
        }
    }, []);

    const topData = React.useMemo(() => {
        return profileData && isFirstTopRender ? calculateTopForTable(profileData.rows, profileData.stringTable.length, { rootCoords: [0, 0], omitted: [] }) : null;
    }, [profileData, isFirstTopRender]);

    const totalFrames = profileData?.rows.reduce((a, row) => a + row.length, 0);

    React.useEffect(() => {
        if (totalFrames !== undefined) {
            uiFactory().rum()?.logInt?.(`${RUM_BASE_NAME}-total-frames`, totalFrames);
        }
    }, [totalFrames]);

    let content: React.JSX.Element | undefined;

    if (props.loading) {
        content = <Loader />;
    } else {
        const flamegraphProps: FlamegraphProps = {
            profileData,
            getState: getQuery,
            setState: setQuery,
            onFinishRendering: (opts) => {
                const size = getFlamegraphSize(totalFrames ?? 0);
                uiFactory().rum()?.finishDataRendering?.('task-flamegraph');
                const memory = measureBrowserMemory();
                function sendWithMetric(metricId: string) {
                    if (opts?.delta && opts?.textNodesCount) {
                        const additional = { textNodesCount: opts.textNodesCount, exceededLimit: opts.exceededLimit,
                            ...(memory ? memory : {}),
                        };
                        uiFactory().rum()?.sendDelta?.(metricId, opts.delta, { additional });
                        uiFactory().rum()?.logInt?.(`${metricId}-nodes`, opts.textNodesCount);
                        uiFactory().rum()?.logInt?.(`${metricId}-total-frames`, totalFrames!);
                        if (memory) {
                            uiFactory().rum()?.logMemory?.(metricId, memory);
                        }
                    }
                }
                sendWithMetric(RUM_BASE_NAME);
                sendWithMetric(RUM_BASE_NAME + '-' + size);
            },
            onSuccess: createSuccessToast,
            goToDefinitionHref: uiFactory().goToDefinitionHref,
            ...props,
        };
        const topTableProps: TopTableProps | null = topData && profileData ? {
            topData,
            profileData,
            navigate,
            getState: getQuery,
            setState: setQuery,
            onFinishRendering: () => {
                uiFactory().rum()?.finishDataRendering?.('top-table');
                const memory = measureBrowserMemory();
                if (memory) {
                    uiFactory().rum()?.logMemory?.('top-table', memory);
                }
            },
            goToDefinitionHref: uiFactory().goToDefinitionHref,
            ...props,
        } : null;

        if (tab === 'flame' ) {
            content = <Flamegraph
                {...flamegraphProps}
            />;
        }
        if (tab === 'top' && topTableProps) {
            content = <TopTable
                {...topTableProps}
            />;
        }
        if ( tab === 'sbs' && topTableProps) {
            content = <SideBySide
                {...flamegraphProps}
                navigate={navigate}
            />;
        }
    }

    return <React.Fragment>
        <Tabs
            className={'visualisation_tabs'}
            activeTab={tab}
            wrapTo={(item, node) => <Link key={item.id} href={`?tab=${item?.id}`}>{node}</Link>}
            items={[
                { id: 'flame', title: 'Flamegraph' },
                { id: 'top', title: 'Top' },
                { id: 'sbs', title: <>Side by side <Beta/></> },
            ]}
            onSelectTab={(newTab: Tab) => {
                if (newTab === 'sbs') {
                    setEnabled(true);
                }
                else {
                    setEnabled(false);
                }
            }}
        />
        {content}
    </React.Fragment>;
};
