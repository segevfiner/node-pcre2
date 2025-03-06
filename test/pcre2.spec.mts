import { describe, expect, test } from "vitest";
import { PCRE2, pcre2 } from "..";

function createMatchArray(
  matches: string[],
  fields: {
    index: number;
    input: string;
    groups?: {
      [key: string]: string;
    };
    indices?: RegExpIndicesArray;
  }
): RegExpExecArray {
  const arr: RegExpExecArray = [...matches] as RegExpExecArray;
  arr.index = fields.index;
  arr.input = fields.input;
  arr.groups = fields.groups;
  if (fields.indices != null) {
    arr.indices = fields.indices;
  }
  return arr;
}

describe("PCRE2 constructor", () => {
  test("single argument", () => {
    const re = new PCRE2("abc");
    expect(re.source).toBe("abc");
    expect(re.flags).toBe("");
    expect(re.global).toBe(false);
    expect(re.ignoreCase).toBe(false);
    expect(re.multiline).toBe(false);
    expect(re.sticky).toBe(false);
    expect(re.unicode).toBe(false);
    expect(re.dotAll).toBe(false);
    expect(re.hasIndices).toBe(false);
    expect(re.unicodeSets).toBe(false);
    expect(re.extended).toBe(false);
    expect(re.extendedMore).toBe(false);
    expect(re.noAutoCapture).toBe(false);
    expect(re.pcre2).toBe(false);
  });

  test("two arguments", () => {
    const re = new PCRE2("abc", "i");
    expect(re.source).toBe("abc");
    expect(re.flags).toBe("i");
    expect(re.global).toBe(false);
    expect(re.ignoreCase).toBe(true);
    expect(re.multiline).toBe(false);
    expect(re.sticky).toBe(false);
    expect(re.unicode).toBe(false);
    expect(re.dotAll).toBe(false);
    expect(re.hasIndices).toBe(false);
    expect(re.unicodeSets).toBe(false);
    expect(re.extended).toBe(false);
    expect(re.extendedMore).toBe(false);
    expect(re.noAutoCapture).toBe(false);
    expect(re.pcre2).toBe(false);
  });
});

describe("pcre2 tagged template literal", () => {
  test("no flags", () => {
    const re = pcre2`abc`;
    expect(re.source).toBe("abc");
    expect(re.flags).toBe("");
    expect(re.global).toBe(false);
    expect(re.ignoreCase).toBe(false);
    expect(re.multiline).toBe(false);
    expect(re.sticky).toBe(false);
    expect(re.unicode).toBe(false);
    expect(re.dotAll).toBe(false);
    expect(re.hasIndices).toBe(false);
    expect(re.unicodeSets).toBe(false);
    expect(re.extended).toBe(false);
    expect(re.extendedMore).toBe(false);
    expect(re.noAutoCapture).toBe(false);
    expect(re.pcre2).toBe(false);
  });

  test("with flags", () => {
    const re = pcre2("i")`abc`;
    expect(re.source).toBe("abc");
    expect(re.flags).toBe("i");
    expect(re.global).toBe(false);
    expect(re.ignoreCase).toBe(true);
    expect(re.multiline).toBe(false);
    expect(re.sticky).toBe(false);
    expect(re.unicode).toBe(false);
    expect(re.dotAll).toBe(false);
    expect(re.hasIndices).toBe(false);
    expect(re.unicodeSets).toBe(false);
    expect(re.extended).toBe(false);
    expect(re.extendedMore).toBe(false);
    expect(re.noAutoCapture).toBe(false);
    expect(re.pcre2).toBe(false);
  });
});

describe("exec", () => {
  test("single match", () => {
    const re = pcre2`abc`;
    const result = re.exec("abc");
    expect(result).toStrictEqual(createMatchArray(["abc"], { index: 0, input: "abc" }));
  });

  test("multiple match", () => {
    const re = pcre2("g")`a`;
    const input = "abaac";
    let result = re.exec(input);
    expect(result).toStrictEqual(createMatchArray(["a"], { index: 0, input }));
    result = re.exec(input);
    expect(result).toStrictEqual(createMatchArray(["a"], { index: 2, input }));
    result = re.exec(input);
    expect(result).toStrictEqual(createMatchArray(["a"], { index: 3, input }));
    result = re.exec(input);
    expect(result).toBeNull();
  });

  test("multiple match with indices", () => {
    const re = pcre2("gd")`a`;
    const input = "abaac";
    let result = re.exec(input);
    expect(result).toStrictEqual(createMatchArray(["a"], { index: 0, input, indices: [[0, 1]] }));
    result = re.exec(input);
    expect(result).toStrictEqual(createMatchArray(["a"], { index: 2, input, indices: [[2, 3]] }));
    result = re.exec(input);
    expect(result).toStrictEqual(createMatchArray(["a"], { index: 3, input, indices: [[3, 4]] }));
    result = re.exec(input);
    expect(result).toBeNull();
  });
});
