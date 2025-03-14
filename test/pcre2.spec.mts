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
    groupsIndices?: {
      [key: string]: [number, number];
    };
    indices?: number[][];
  }
): RegExpExecArray {
  const arr: RegExpExecArray = [...matches] as RegExpExecArray;
  arr.index = fields.index;
  arr.input = fields.input;
  arr.groups = createGroupsArray(fields.groups);
  if (fields.indices != null) {
    arr.indices = createIndicesArray(fields.indices);
    arr.indices.groups = createIndicesGroupsArray(fields.groupsIndices);
  }
  return arr;
}

function createGroupsArray(groups?: { [key: string]: string }):
  | {
      [key: string]: string;
    }
  | undefined {
  if (groups == null) {
    return groups;
  }
  const obj = Object.create(null) as {
    [key: string]: string;
  };
  for (const [key, value] of Object.entries(groups)) {
    obj[key] = value;
  }
  return obj;
}

function createIndicesArray(
  indices: number[][],
  fields?: {
    groups?: {
      [key: string]: [number, number];
    };
  }
): RegExpIndicesArray {
  const arr = [...indices] as RegExpIndicesArray;
  arr.groups = fields?.groups;
  return arr;
}

function createIndicesGroupsArray(indicesGroups?: {
  [key: string]: [number, number];
}) {
  if (indicesGroups == null) {
    return undefined;
  }

  const obj = Object.create(null) as {
    [key: string]: [number, number];
  };
  for (const [key, value] of Object.entries(indicesGroups)) {
    obj[key] = value;
  }
  return obj;
}

describe.concurrent("PCRE2 constructor", () => {
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
    expect(re.asciiBsd).toBe(false);
    expect(re.asciiBss).toBe(false);
    expect(re.asciiBsw).toBe(false);
    expect(re.asciiDigit).toBe(false);
    expect(re.asciiPosix).toBe(false);
    expect(re.caselessRestrict).toBe(false);
    expect(re.dupnames).toBe(false);
    expect(re.ungreedy).toBe(false);
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
    expect(re.asciiBsd).toBe(false);
    expect(re.asciiBss).toBe(false);
    expect(re.asciiBsw).toBe(false);
    expect(re.asciiDigit).toBe(false);
    expect(re.asciiPosix).toBe(false);
    expect(re.caselessRestrict).toBe(false);
    expect(re.dupnames).toBe(false);
    expect(re.ungreedy).toBe(false);
    expect(re.pcre2).toBe(false);
  });
});

describe.concurrent("pcre2 tagged template literal", () => {
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
    expect(re.asciiBsd).toBe(false);
    expect(re.asciiBss).toBe(false);
    expect(re.asciiBsw).toBe(false);
    expect(re.asciiDigit).toBe(false);
    expect(re.asciiPosix).toBe(false);
    expect(re.caselessRestrict).toBe(false);
    expect(re.dupnames).toBe(false);
    expect(re.ungreedy).toBe(false);
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
    expect(re.asciiBsd).toBe(false);
    expect(re.asciiBss).toBe(false);
    expect(re.asciiBsw).toBe(false);
    expect(re.asciiDigit).toBe(false);
    expect(re.asciiPosix).toBe(false);
    expect(re.caselessRestrict).toBe(false);
    expect(re.dupnames).toBe(false);
    expect(re.ungreedy).toBe(false);
    expect(re.pcre2).toBe(false);
  });
});

describe.concurrent("exec", () => {
  test("single match", () => {
    const re = pcre2`abc`;
    const input = "abc";
    const result = re.exec(input);
    expect(result).toStrictEqual(
      createMatchArray(["abc"], { index: 0, input })
    );
  });

  test("multiple times", () => {
    const re = pcre2`abc`;

    let input = "abc";
    let result = re.exec(input);
    expect(result).toStrictEqual(
      createMatchArray(["abc"], { index: 0, input })
    );

    input = "foo";
    result = re.exec(input);
    expect(result).toBeNull();

    input = "fooabcbar";
    result = re.exec(input);
    expect(result).toStrictEqual(
      createMatchArray(["abc"], { index: 3, input })
    );
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
    expect(result).toStrictEqual(
      createMatchArray(["a"], {
        index: 0,
        input,
        indices: [[0, 1]],
      })
    );
    result = re.exec(input);
    expect(result).toStrictEqual(
      createMatchArray(["a"], {
        index: 2,
        input,
        indices: [[2, 3]],
      })
    );
    result = re.exec(input);
    expect(result).toStrictEqual(
      createMatchArray(["a"], {
        index: 3,
        input,
        indices: [[3, 4]],
      })
    );
    result = re.exec(input);
    expect(result).toBeNull();
  });

  test("single match with captures", () => {
    const re = pcre2`^foo(bar)qux: (\d+)$`;
    const input = "foobarqux: 123";
    const result = re.exec(input);
    expect(result).toStrictEqual(
      createMatchArray(["foobarqux: 123", "bar", "123"], { index: 0, input })
    );
  });

  test("single match with named captures", () => {
    const re = pcre2`^foo(?<one>bar)qux: (?<value>\d+)$`;
    const input = "foobarqux: 123";
    const result = re.exec(input);
    expect(result).toStrictEqual(
      createMatchArray(["foobarqux: 123", "bar", "123"], {
        index: 0,
        input,
        groups: { one: "bar", value: "123" },
      })
    );
  });

  test("single match with named captures", () => {
    const re = pcre2`^foo(?<one>bar)qux: (?<value>\d+)$`;
    const input = "foobarqux: 123";
    const result = re.exec(input);
    expect(result).toStrictEqual(
      createMatchArray(["foobarqux: 123", "bar", "123"], {
        index: 0,
        input,
        groups: { one: "bar", value: "123" },
      })
    );
  });

  test("single match with named captures and indices", () => {
    const re = pcre2("d")`^foo(?<one>bar)qux: (?<value>\d+)$`;
    const input = "foobarqux: 123";
    const result = re.exec(input);
    expect(result).toStrictEqual(
      createMatchArray(["foobarqux: 123", "bar", "123"], {
        index: 0,
        input,
        groups: { one: "bar", value: "123" },
        indices: [
          [0, 14],
          [3, 6],
          [11, 14],
        ],
        groupsIndices: { one: [3, 6], value: [11, 14] },
      })
    );
  });
});

describe.concurrent("test", () => {
  test("single match", () => {
    const re = pcre2`abc`;
    expect(re.test("abc")).toBe(true);
    expect(re.test("foo")).toBe(false);
  });

  test("multiple match no global", () => {
    const re = pcre2`abc`;
    const input = "abcfooabc";
    expect(re.test(input)).toBe(true);
    expect(re.test(input)).toBe(true);
    expect(re.test(input)).toBe(true);
  });

  test("multiple match global", () => {
    const re = pcre2("g")`abc`;
    const input = "abcfooabc";
    expect(re.test(input)).toBe(true);
    expect(re.test(input)).toBe(true);
    expect(re.test(input)).toBe(false);
  });

  test("multiple match sticky", () => {
    const re = pcre2("y")`abc`;
    const input = "abcfooabc";
    expect(re.test(input)).toBe(true);
    expect(re.test(input)).toBe(false);
    expect(re.test(input)).toBe(true);
  });
});

describe.concurrent("match", () => {
  test("single match", () => {
    const re = pcre2`abc`;
    const input = "abcfooabc";
    const result = input.match(re);
    expect(result).toStrictEqual(
      createMatchArray(["abc"], { index: 0, input })
    );
  });

  test("multiple match", () => {
    const re = pcre2("g")`abc`;
    const input = "abcfooabc";
    const result = input.match(re);
    expect(result).toStrictEqual(
      ["abc", "abc"]
    );
  })
});
