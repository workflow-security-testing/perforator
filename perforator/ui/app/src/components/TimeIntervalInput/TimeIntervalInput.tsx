import React from 'react';

import type { RangeDateSelectionProps, RangeValue } from '@gravity-ui/date-components';
import { RangeDateSelection } from '@gravity-ui/date-components';
import type { DateTime } from '@gravity-ui/date-utils';
import { Check, Xmark } from '@gravity-ui/icons';
import { Button, Icon } from '@gravity-ui/uikit';

import { uiFactory } from 'src/factory';
import { cn } from 'src/utils/cn';

import { areIntervalsEqual, parseTimeInterval, type TimeInterval } from './TimeInterval';
import { TimeIntervalControls } from './TimeIntervalControls/TimeIntervalControls';

import './TimeIntervalInput.scss';


export type { TimeInterval } from './TimeInterval';


const MIN_SELECTION_PRECISION = 1;  // 1 millisecond
const MIN_SELECTION_DURATION = 5 * 1000;  // 5 seconds
const MAX_SELECTION_DURATION = 365 * 24 * 60 * 60 * 100;  // 1 year

export interface TimeIntervalInputProps extends Pick<RangeDateSelectionProps, 'numberOfIntervals'> {
    initInterval: TimeInterval;
    onUpdate: (range: TimeInterval) => void;
    className?: string;
    headerControls?: boolean;
    isEditing?: boolean;
    onSave?: () => void;
}

const b = cn('time-interval-selector');

export const TimeIntervalInput: React.FC<TimeIntervalInputProps> = props => {
    const [interval, setInterval] = React.useState(props.initInterval);

    const handleUpdate = React.useCallback((newInterval: TimeInterval) => {
        props.onUpdate(newInterval);
        setInterval(newInterval);
    }, [props.onUpdate, setInterval]);

    const handleRangeUpdate = React.useCallback((range: RangeValue<DateTime>) => {
        handleUpdate({
            start: range.start.toISOString(),
            end: range.end.toISOString(),
        });
    }, [handleUpdate]);


    const handleCancel = React.useCallback(() => {
        props.onUpdate(props.initInterval);
        setInterval(props.initInterval);
    }, [props.onUpdate, props.initInterval, setInterval]);

    const className = b(
        {
            gravity: uiFactory().gravityStyles(),
        },
        props.className,
    );

    const hasSelectionChanged = areIntervalsEqual(interval, props.initInterval);

    return (
        <div className={className}>
            <TimeIntervalControls
                interval={interval}
                onUpdate={handleUpdate}
                header={props.headerControls}
            />
            <RangeDateSelection
                className="time-interval-selector__ruler"
                displayNow
                hasScaleButtons
                minDuration={MIN_SELECTION_DURATION}
                maxDuration={MAX_SELECTION_DURATION}
                align={MIN_SELECTION_PRECISION}
                scaleButtonsPosition="end"
                value={parseTimeInterval(interval)}
                onUpdate={handleRangeUpdate}
                numberOfIntervals={props.numberOfIntervals}
            />
            {props.isEditing && <>
                <Button className={b('button')} disabled={hasSelectionChanged} onClick={handleCancel}><Icon data={Xmark}/></Button>
                <Button className={b('button')} view="action" disabled={hasSelectionChanged} onClick={props.onSave}><Icon data={Check}/></Button>
            </>}
        </div>
    );
};
