# @segevfiner/pcre2
[![NPM Version](https://img.shields.io/npm/v/%40segevfiner%2Fpcre2)](https://www.npmjs.com/package/@segevfiner/pcre2)
[![CI](https://github.com/segevfiner/node-pcre2/actions/workflows/ci.yml/badge.svg)](https://github.com/segevfiner/node-pcre2/actions/workflows/ci.yml)

Node-API bindings to [PCRE2](https://pcre2project.github.io/pcre2/).

Install using:
```sh
npm/yarn/pnpm add @segevfiner/pcre2
```

## Usage

Create an instance using the `PCRE2` constructor or `pcre2` tagged template literal:
```ts
import { PCRE2, pcre2 } from "pcre2";

const re1 = new PCRE2("a(b)c");
const re2 = pcre2`a(b)c`;

const re3 = new PCRE2("foo(bar)+", "g");
const re4 = pcre2("g")`foo(bar)+`; // With flags
```

The template literal can be called and then used a a literal to add flags as
shown above (e.g. `` pcre2("g")`foo(bar)+` ``).

The `PCRE2` object has the same interface as the builtin JavaScript [`RegExp`]
object, and can be used with the string methods taking a `RegExp` as well (Such
as `match`, `matchAll`, `replace`, etc.).

By default, the library enables a few PCRE2 options to more closely behave like
JavaScript's builtin `RegExp`, namely: `PCRE2_ALT_BSUX`, `PCRE2_DOLLAR_ENDONLY`, `PCRE2_MATCH_UNSET_BACKREF`, `PCRE2_EXTRA_ALT_BSUX`, and the behavior on empty matches is the one specified by JavaScript rather than `pcre2grep`, this can be toggled using the `p` (`pcre2Mode`) flag.

Additional flags supported by PCRE2 are supported (See [PCRE2 docs] for details):
* `x` (`extended`) - Ignores whitespace in the pattern and allows comments.
* `xx` (`extendedMore`) - Like `extended`, but also ignores unescaped
  space and horizontal tab in character classes.
* `n` (`noAutoCapture`) - Disables the use of numbered capturing groups in the pattern.
* `aD` (`asciiBsd`) - Forces `\d` to match only ASCII digits even with `(*UCP)`.
* `aS` (`asciiBss`) - Forces `\s` to match only ASCII space characters even with `(*UCP)`.
* `aW` (`asciiBsw`) - Forces `\w` to match only ASCII word characters even with `(*UCP)`.
* `aT` (`asciiDigit`) - Forces the POSIX character classes `[:digit:]` and `[:xdigit:]` to match only ASCII digits even with `(*UCP)`.
* `aP` (`asciiPosix`) - Forces all the POSIX character classes, including `[:digit:]` and `[:xdigit:]`, to match only ASCII characters even with `(*UCP)`.
* `a` - Enables all the ascii options above.
* `r` (`caselessRestrict`) - Disables recognition of case-equivalences that cross the ASCII/non-ASCII boundary when `unicode` or `(*UCP)` is enabled.
* `J` (`dupnames`) - Allows duplicate capture group names.
* `U` (`ungreedy`) - Inverts the "greediness" of the quantifiers so that they are not greedy by default, but become greedy if followed by "?".
* `p` (`pcre2Mode`) - Disables JavaScript compatibility options/behaviors to behave like PCRE2.

[`RegExp`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/RegExp
[PCRE2 docs]: https://pcre2project.github.io/pcre2/doc/

## License
BSD-3-Clause.
