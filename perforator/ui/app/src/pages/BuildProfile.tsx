import React from 'react';

import { AxiosError } from 'axios';
import { useNavigate, useSearchParams } from 'react-router-dom';

import { Loader } from '@gravity-ui/uikit';

import { ErrorPanel } from 'src/components/ErrorPanel/ErrorPanel';
import type { ProfileTaskQuery } from 'src/models/Task';
import { useUserSettings } from 'src/providers/UserSettingsProvider';
import {
    defaultProfileTaskQuery,
    startProfileTask,
} from 'src/utils/profileTask';


const setupQuery = (searchParams: URLSearchParams): ProfileTaskQuery => {
    const query = defaultProfileTaskQuery();
    searchParams.forEach((value, key) => {
        (query as any)[key] = value ?? query[key as keyof ProfileTaskQuery];
    });
    return query;
};

export interface BuildProfileProps {}

const WELL_KNOWN_QUERY_PARAMS = [
    'flamegraphQuery',
    'exactMatch',
];

function preserveWellKnownQueryParams(searchParams: URLSearchParams): URLSearchParams {
    const preserved = new URLSearchParams();
    searchParams.forEach((value, key) => {
        if (WELL_KNOWN_QUERY_PARAMS.includes(key)) {
            preserved.set(key, value);
        }
    });
    return preserved;
}


export const BuildProfile: React.FC<BuildProfileProps> = () => {
    const isMounted = React.useRef(false);
    const [error, setError] = React.useState<string | undefined>(undefined);
    const { userSettings: { showPrettyPythonFrames } } = useUserSettings();
    const [searchParams] = useSearchParams();
    const navigate = useNavigate();

    const navigateToTask = React.useCallback(async () => {
        const query = setupQuery(searchParams);
        try {

            const taskId = await startProfileTask(query, { showPrettyPythonFrames: showPrettyPythonFrames });
            const q = preserveWellKnownQueryParams(new URLSearchParams(window.location.search));
            navigate(`/task/${taskId}?${q.toString()}`, { replace: true });
        } catch (e) {
            if (e instanceof AxiosError) {
                setError(e.message);
            } else {
                setError((e as any)?.message ?? 'Unknown error');
            }
        }
    }, [navigate, searchParams, showPrettyPythonFrames]);

    React.useEffect(() => {
        if (!isMounted.current) {
            navigateToTask();
            isMounted.current = true;
        }
    }, []);

    return error ? <ErrorPanel message={error} /> : <Loader />;
};
