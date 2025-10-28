import { useLayoutEffect, useState } from 'react';

import { CircleCheck } from '@gravity-ui/icons';
import { Icon, Text } from '@gravity-ui/uikit';

import { cn } from 'src/utils/cn';
import { controller } from 'src/utils/onboarding';

import { Link } from '../Link/Link';

import { TUTORIALS_LIST } from './utils';

import './Tutorials.css';


export type TutorialsProps = {
    onItemClick: () => void;
};

const b = cn('aside-tutorials');

export const Tutorials: React.FC<TutorialsProps> = ({ onItemClick }) => {
    const [, setHasEnsuredRunning] = useState(false);
    const ensureRunning = async () => controller.ensureRunning().then(() => setHasEnsuredRunning(true));
    useLayoutEffect(() => {
        ensureRunning();
    }, []);

    return (
        <div>
            {TUTORIALS_LIST.map((item) => {
                const isPassed = controller.state.progress?.finishedPresets.some((preset) => preset === item.slug);
                // the onboarding library does not allow to rerun the preset directly, so instead we will reset it
                const handleMaybeOldClick = () => {
                    if (item.slug && isPassed) {
                        controller.resetPresetProgress(item.slug);
                    }
                    onItemClick();
                };
                return (
                    <Link key={item.slug} view="primary" onClick={handleMaybeOldClick} href={item.href ?? ''}>
                        <div className={b('item')}>
                            {item.index ? `${item.index}.` : null}
                            {isPassed ? <Icon data={CircleCheck} className={b('icon')} /> : null}
                            <Text variant="body-1">{item.title}</Text>
                        </div>
                    </Link>
                );})}
        </div>
    );
};
