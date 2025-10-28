import type { OnboardingNames } from 'src/utils/onboarding';


type Tutorial = {
    title?: string;
    slug?: OnboardingNames;
    passed?: boolean;
    index?: number;
    href?: string;
};

export const enrichTutorialsForView = (list: Tutorial[]) => list.map((item, index) => ({
    ...item,
    index: index + 1,
    href: '/tutorials/' + item.slug,
})) as Tutorial[];

export const TUTORIALS_LIST = enrichTutorialsForView([
    {
        title: 'Basics of flamegraph navigation',
        slug: 'basics',
    },
] as Tutorial[]);
