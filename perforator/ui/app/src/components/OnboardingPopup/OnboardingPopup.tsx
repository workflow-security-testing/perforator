import { shift, size } from '@floating-ui/react-dom';

import { Button, Popup, Text } from '@gravity-ui/uikit';

import { useOnboardingHint } from 'src/utils/onboarding';

import './OnboardingPopup.css';


export const OnboardingPopup = () => {
    const { anchorRef, hint, open } = useOnboardingHint();

    const floatingMiddlewares = [
        shift({
            boundary: anchorRef?.current!,
            crossAxis: true,
        }),
        size({
            apply: ({ availableWidth, elements }: any) => {
                const value = `${Math.max(0, availableWidth)}px`;
                elements.floating.style.maxWidth = value;
            },
            boundary: anchorRef?.current!,
        }),
    ];

    return (
        <>
            <Popup
                floatingMiddlewares={floatingMiddlewares}
                anchorElement={anchorRef.current}
                open={open}
                style={{ padding: '4px 8px 8px' }}
                disableEscapeKeyDown
            >
                <Text as="div" variant="subheader-2">{hint?.step.name}</Text>
                {hint?.step.description}
                <div className="onboarding-hint__actions">
                    {hint?.step.hintParams?.actions?.map((action, i) => {
                        return (
                            <Button
                                className="onboarding-hint__action-button"
                                key={i}
                                onClick={action.onClick}
                                view={action.view}
                            >
                                {action.children}
                            </Button>
                        );
                    })}
                </div>
                <Text className="onboarding-hint__index" as="p" variant="caption-1">{hint?.step.hintParams?.index}</Text>
            </Popup>
        </>
    );
};
