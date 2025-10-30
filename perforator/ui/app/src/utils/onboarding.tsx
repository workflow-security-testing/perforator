import type { Coordinate } from '@perforator/flamegraph';

import type { PresetStep } from '@gravity-ui/onboarding';
import { createOnboarding, createPreset } from '@gravity-ui/onboarding';
import { Text } from '@gravity-ui/uikit';

import { ColorSwatch } from 'src/components/ColorSwatch/ColorSwatch';
import { LocalStorageKey } from 'src/const/localStorage';

import { createSuccessToast } from './toaster';


export const enum FlamegraphSteps {
    FlamegraphOverview = 'flamegraph-overview',
    FlamegraphClick = 'flamegraph-click',
    FlamegraphAltClick = 'flamegraph-alt-click',
    GoBack = 'go-back',
    ResetOmit = 'reset-omit',
    Search = 'search',
    SearchReset = 'search-reset',
    ShowMatchedStacks = 'show-matched-stacks',
    LeftHeavy = 'left-heavy',
    Final = 'final'
}

export const enum OnboardingNames {
    Basics = 'basics'
}

const progressSuccessToastHooks = {
    onStepPass: () => {
        createSuccessToast({ name: 'step', content: 'Step passed' });
    },
};

const setIndexes: <
    I extends { hintParams?: H },
    H = I extends { hintParams?: infer Hint } ? Hint : never,
>(
    item: I,
    i: number,
    length: number,
) => I & { hintParams: H & { index?: string } } = (item, i, length) => ({
    ...item,
    hintParams: { ...item.hintParams, index: i !== 0 ? `${i} out of ${length} passed` : undefined },
});


export const demoFlamegraphPreset = createPreset(
    ({ goNextStep, goPrevStep: _goPrevStep }) => {
        const steps = [
            {
                slug: FlamegraphSteps.FlamegraphOverview,
                name: 'Flamegraph overview',
                description: '',
                hintParams: {
                    children:
                    <>This is a flamegraph. It includes both the library code and your code, it also includes the kernel-space functions.
            The code is by default shades of <ColorSwatch color={'rgb(205, 0, 0)'}/> orange, the kernel-space code is shades of <ColorSwatch color={'rgb(96, 96, 205)'}/> blue. For some languages with specialised instrumentation and first-class support we use special colors, like <ColorSwatch color="rgb(103, 178, 120)"/> green for python.</>,
                    actions: [
                        {
                            children: 'Go next',
                            view: 'action' as const,
                            onClick: () => {
                                goNextStep();
                            },
                        },
                    ],
                },
            },
            {
                slug: FlamegraphSteps.FlamegraphClick,
                name: 'Flamegraph click',
                description: '',
                hintParams: {
                    children:
                        <>Let's try moving around flamegraph a bit. Click <Text variant={'code-1'}>inefficient_calc_sum</Text> rectangle to stretch it to 100%.</>,
                    highlightCoordinate: [8, 0] as Coordinate,
                },
                hooks: progressSuccessToastHooks,
            },
            {
                slug: FlamegraphSteps.GoBack,
                name: 'Going back',
                description:
                    'Now let\'s get back to the general overview: to return there click on the root node here',
                hintParams: {
                    highlightCoordinate: [0, 0] as Coordinate,
                },
                hooks: progressSuccessToastHooks,
            },
            {
                slug: FlamegraphSteps.FlamegraphAltClick,
                name: 'Flamegraph alt click',
                description: '',
                hintParams: {
                    children: <>We can use context menu to omit unneeded nodes like this function <Text variant={'code-1'}>shuffle_some_array</Text>.<br/> It uses standard library, probably efficient enough not to focus on it<br/> Let's click it with right mouse button and delete it.</>,
                    highlightCoordinate: [8, 1] as Coordinate,
                },
                hooks: progressSuccessToastHooks,
            },
            {
                slug: FlamegraphSteps.ResetOmit,
                name: 'Reset omit',
                description:
                    'We\'ve omitted some nodes on previous steps. Let\'s reset them and return the flamegraph back to its original form with the button to the right',
                hintParams: {
                    className: '.flamegraph__clear-deletion',
                },
                hooks: progressSuccessToastHooks,
            },
            {
                slug: FlamegraphSteps.Search,
                name: 'Search',
                description: '',
                hintParams: {
                    children:
                        <>We can search for a node by its name. Let's search for "<Text variant="code-1">kernel</Text>"</>,
                    className: '.flamegraph__button_search',
                },
                hooks: progressSuccessToastHooks,
            },
            {
                slug: FlamegraphSteps.ShowMatchedStacks,
                name: 'Show matched stacks',
                hintParams: {
                    className: '.flamegraph__button_keep-only-found',
                },
                description:
                    'Let\'s show only matched stacks and their parents with the "show matched stacks" button',
                hooks: progressSuccessToastHooks,
            },
            {
                slug: FlamegraphSteps.SearchReset,
                name: 'Search reset',
                hintParams: {
                    className: '.flamegraph__clear',
                },
                description:
                    'Let\'s reset the search with this button to the right (you can also do it with Esc)',
                hooks: progressSuccessToastHooks,
            },
            {
                slug: FlamegraphSteps.LeftHeavy,
                name: 'Left heavy',
                description:
                    'Let\'s try the left heavy form of the flamegraph. It will reorder the flame by the number of events.',
                hintParams: {
                    className: '.flamegraph__switch_left-heavy',
                },
                hooks: progressSuccessToastHooks,
            },
            {
                slug: FlamegraphSteps.Final,
                name: 'The flamegraph tutorial is now completed',
                description:
                    'Congratulations, you\'ve completed the flamegraph tutorial.',
                hintParams: {
                    actions: [

                        {
                            children: 'Finish',
                            view: 'action' as const,
                            onClick: () => {
                                goNextStep();
                                controller.finishPreset(OnboardingNames.Basics);
                            },
                        },
                    ],
                },
                hooks: progressSuccessToastHooks,
            },
        ] satisfies PresetStep<FlamegraphSteps, any>[];

        return {
            enabled: true,
            name: OnboardingNames.Basics,
            visibility: 'visible',
            steps: steps.map((step, i) => setIndexes(step, i, steps.length - 1)),
        };
    },
);

function getFromLocalStorageAndParse(key: string) {
    try {
        return JSON.parse(localStorage.getItem(key) ?? '{}');
    } catch {
        return {};
    }
}

export const { useOnboardingHint, useOnboardingStep, useOnboardingPresets, presetsNames, useWizard, controller } = createOnboarding({
    baseState: {
        ...(getFromLocalStorageAndParse(LocalStorageKey.TutorialBase)),
        enabled: true,
    },
    config: {
        presets: {
            [OnboardingNames.Basics]: demoFlamegraphPreset,
        },
    },
    getProgressState: () => getFromLocalStorageAndParse(LocalStorageKey.TutorialProgress),
    progressState: getFromLocalStorageAndParse(LocalStorageKey.TutorialProgress),
    onSave: {
        state: async (state) => { localStorage.setItem(LocalStorageKey.TutorialBase, JSON.stringify(state)); },
        progress: async (progress) => { localStorage.setItem(LocalStorageKey.TutorialProgress, JSON.stringify(progress)); },
    },
    debugMode: true,
    logger: {
        level: 'debug',
    },

});

export function useWellKnownHint(preset: typeof presetsNames, step?: string) {
    const hint = useOnboardingHint();
    const currentPreset = hint.hint?.preset;
    const currentStep = hint.hint?.step;

    if (preset === currentPreset && (!step || currentStep?.slug === step)) {
        return hint;
    }
    return undefined;
}
