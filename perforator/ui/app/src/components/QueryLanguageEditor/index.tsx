import React from 'react';

import { QuerySuggestProvider } from 'src/providers/QuerySuggestProvider';

import type { QueryLanguageEditorProps } from './QueryLanguageEditor';

import './QueryLanguageEditor.scss';


const QueryLanguageEditorImpl = React.lazy(() => import('./QueryLanguageEditor').then(i => ({ default: i.QueryLanguageEditorImpl })));

const QueryLanguageFallback: React.FC<{selector?: string; className?: string}> = ({ selector, className }) => (
    <code className={'selector-input__skeleton' + (className ? ' ' + className : '')}>{selector}</code>
);

export const QueryLanguageEditor: React.FC<QueryLanguageEditorProps> = props => (
    <QuerySuggestProvider>
        <React.Suspense fallback={<QueryLanguageFallback className={props.className} selector={props.selector}/>}>
            <QueryLanguageEditorImpl {...props} />
        </React.Suspense>
    </QuerySuggestProvider>
);

export type { QueryLanguageEditorProps } from './QueryLanguageEditor';
