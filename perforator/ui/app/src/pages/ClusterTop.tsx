import { useCallback, useEffect, useMemo } from 'react';

import { Loader, Select, type SelectOption } from '@gravity-ui/uikit';

import { ClusterTopTable } from 'src/components/ClusterTopTable/ClusterTopTable';
import { countHoursInterval } from 'src/components/ClusterTopTable/utils';
import { ErrorPanel } from 'src/components/ErrorPanel/ErrorPanel';
import { useAsyncResult } from 'src/components/TaskReport/TaskFlamegraph/useFetchResult';
import { apiClient } from 'src/utils/api';
import { useTypedQuery } from 'src/utils/query';

import type { Page } from './Page';


export const ClusterTop: Page = () => {
    const [getQuery, setQuery] = useTypedQuery<'generation'>();
    const currentGeneration = getQuery('generation', '') ?? '';
    const setGeneration = (value: string) => setQuery({ generation: value });
    const getData = useCallback(() => {
        return apiClient
            .getGenerations(null, {}).then(value => value.data);
    }, []);
    const { error, data: generations, loading } = useAsyncResult({ getData: getData });
    const options = useMemo<SelectOption[]>(() => {
        return (
            generations?.Generations.map((gen) => ({
                content: `${gen.ID}: ${gen.From} - ${gen.To}`,
                value: String(gen.ID),
            } as SelectOption)) ?? []
        );
    }, [generations]);
    const currentGenerationObject = useMemo(
        () => generations?.Generations.find(({ ID }) => ID === Number(currentGeneration)),
        [currentGeneration, generations?.Generations],
    );

    useEffect(() => {
        if (currentGeneration === '' && !loading && !error && (generations?.Generations?.length ?? 0) > 0) {
            setGeneration(String(generations?.Generations[0].ID));
        }
    }, [currentGeneration, error, generations?.Generations, loading, setGeneration]);

    const timeInterval = currentGenerationObject ? countHoursInterval(currentGenerationObject) : null;

    if (error) {
        return <ErrorPanel message={error?.message}/>;
    }

    if (loading) {
        return <Loader/>;
    }

    return (
        <>
            <div>Cluster Top</div>
            <Select
                options={options}
                value={[currentGeneration]}
                onUpdate={([val]) => setGeneration(val)}
                placeholder={'generation'}
            />
            {currentGeneration && currentGenerationObject && timeInterval && <ClusterTopTable generation={Number(currentGeneration)} timeInterval={timeInterval}/>}
        </>
    );
};
