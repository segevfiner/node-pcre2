import bindings from 'bindings';

// eslint-disable-next-line @typescript-eslint/no-unused-vars
declare namespace Addon {
    class PCRE2 {
        constructor(pattern: RegExp | PCRE2 | string, flags?: string);

        /**
         * Executes a search on a string using a regular expression pattern, and returns an array containing the results of that search.
         * @param string The String object or string literal on which to perform the search.
         */
        exec(string: string): RegExpExecArray | null;

        /**
         * Returns a Boolean value that indicates whether or not a pattern exists in a searched string.
         * @param string String on which to perform the search.
         */
        test(string: string): boolean;

        /** Returns a copy of the text of the regular expression pattern. Read-only. The regExp argument is a Regular expression object. It can be a variable name or a literal. */
        readonly source: string;
    }
}

export const { PCRE2 } = bindings('pcre2.node') as typeof Addon;
