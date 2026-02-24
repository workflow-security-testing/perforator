import React from 'react';

import { useNavigate } from 'react-router-dom';

import { dateTimeParse } from '@gravity-ui/date-utils';

import { uiFactory } from 'src/factory';
import type { TaskResult } from 'src/models/Task';
import { redirectToTaskPage } from 'src/utils/profileTask';
import { cutIdFromSelector, cutTimeFromSelector, parseTimestampFromSelector } from 'src/utils/selector';

import type { TimeInterval } from '../TimeIntervalInput/TimeInterval';
import { TimeIntervalInput } from '../TimeIntervalInput/TimeIntervalInput';


export const EditableTaskTimeInterval: React.FC<{ task: TaskResult | null }> = ({ task }) => {
    const spec = task?.Spec?.MergeProfiles;
    const query = spec?.Query;
    const selector = query?.Selector;
    const maxSamples = query?.MaxSamples || spec?.MaxSamples as number;
    const [timelineValue, setTimelineValue] = React.useState<TimeInterval | null>(null);
    const navigate = useNavigate();
    const isSingleProfile = maxSamples === 1;
    const time = React.useMemo<TimeInterval | null>(() => {
        const baseTime = selector ? parseTimestampFromSelector(selector!) : null;

        if (!baseTime?.from || !baseTime?.to) {
            return null;
        }

        if (isSingleProfile) {
            return {
                start: dateTimeParse(baseTime?.from)?.subtract(5, 'm').toISOString(),
                end: dateTimeParse(baseTime?.to)?.add(5, 'm').toISOString(),
            } as TimeInterval;
        }
        else {
            return {
                start: baseTime.from,
                end: baseTime.to,
            } as TimeInterval;
        }

    }, [isSingleProfile, selector]);

    const handleSave = React.useCallback(() => {
        if (!selector) {
            return;
        }
        let newSelector = cutTimeFromSelector(selector);
        if (isSingleProfile) {
            newSelector = cutIdFromSelector(newSelector);
        }
        if (timelineValue) {
            uiFactory().reachGoal('EDIT_TASK_TIMELINE');
            redirectToTaskPage(navigate, { selector: newSelector, maxProfiles: maxSamples, from: timelineValue.start, to: timelineValue.end });
        }
    }, [isSingleProfile, maxSamples, navigate, selector, timelineValue]);

    if (!task) {
        return null;
    }

    return time ? <TimeIntervalInput
        isEditing={true}
        initInterval={time}
        onUpdate={setTimelineValue}
        onSave={handleSave}
        numberOfIntervals={isSingleProfile ? 10 : undefined} /> : null;
};
