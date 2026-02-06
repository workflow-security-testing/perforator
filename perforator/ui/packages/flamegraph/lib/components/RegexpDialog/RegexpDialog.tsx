import * as React from 'react';
import { useCallback, useEffect, useState } from 'react';

import { Magnifier } from '@gravity-ui/icons';
import { Checkbox, Dialog, Icon, TextInput } from '@gravity-ui/uikit';

import { useRegexError } from './useRegexError';

import './RegexpDialog.css';


export type SearchUpdate = {text: string; exactMatch?: boolean; excludeText: string}

interface RegexpDialogProps {
    showDialog: boolean;
     onCloseDialog: () => void;
     initialSearch?: string | null;
     initialExact?: boolean | null;
     initialExcludeText?: string | null;
     onSearchUpdate: (update: SearchUpdate) => void;
}

export function RegexpDialog({ showDialog, onCloseDialog, onSearchUpdate, initialExact, initialSearch, initialExcludeText }: RegexpDialogProps) {
    const [searchQuery, setSearchQuery] = useState(initialSearch ?? '');
    const [excludeText, setExcludeText] = useState(initialExcludeText ?? '');
    const [exact, setExact] = useState(initialExact ?? false);
    const controlRef = React.useRef<null | HTMLInputElement>(null);
    const regexError = useRegexError(searchQuery);
    const excludeRegexError = useRegexError(excludeText);

    const handleKeyDown = useCallback((e: KeyboardEvent) => {
        if (e.key === 'Enter' && !regexError && !excludeRegexError) {
            onSearchUpdate({ text: searchQuery, exactMatch: exact, excludeText });
            e.preventDefault();
        }
    }, [exact, onSearchUpdate, regexError, excludeRegexError, searchQuery, excludeText]);

    const handleApply = () => {
        if (regexError || excludeRegexError) {
            return;
        }

        onSearchUpdate({ text: searchQuery, exactMatch: exact, excludeText });

    };

    useEffect(() => {
        document.addEventListener('keydown', handleKeyDown);
        return () => document.removeEventListener('keydown', handleKeyDown);
    }, [handleKeyDown]);

    const handleSearchUpdate = (str: string) => {
        setSearchQuery(str);
    };

    const handleExcludeUpdate = (str: string) => {
        setExcludeText(str);
    };

    return (
        <Dialog initialFocus={controlRef} className="regexp-dialog__dialog" size="l" open={showDialog} onClose={onCloseDialog}>
            <Dialog.Header insertBefore={<Icon className="regexp-dialog__header-icon" data={Magnifier}/>} caption="Search"/>
            <Dialog.Body>
                <TextInput
                    note={'Regular expressions are supported'}
                    autoFocus={true}
                    controlRef={controlRef}
                    value={searchQuery}
                    onUpdate={handleSearchUpdate}
                    error={Boolean(regexError)}
                    errorMessage={regexError} />
                <TextInput
                    note={'Exclude pattern (optional)'}
                    value={excludeText}
                    onUpdate={handleExcludeUpdate}
                    error={Boolean(excludeRegexError)}
                    errorMessage={excludeRegexError} />
                <Checkbox title="Disable regex parsing, literal mode" checked={exact} onUpdate={setExact}>Exact match</Checkbox>
            </Dialog.Body>
            <Dialog.Footer
                onClickButtonCancel={onCloseDialog}
                textButtonCancel="Cancel"
                propsButtonApply={{ disabled: Boolean(regexError || excludeRegexError) }}
                onClickButtonApply={handleApply}
                textButtonApply={'Search'}
            />
        </Dialog>
    );
}
