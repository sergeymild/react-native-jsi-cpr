'use strict';

function encode(val: string) {
  return encodeURIComponent(val)
    .replace(/%3A/gi, ':')
    .replace(/%24/g, '$')
    .replace(/%2C/gi, ',')
    .replace(/%20/g, '+')
    .replace(/%5B/gi, '[')
    .replace(/%5D/gi, ']');
}

/**
 * Determine if a value is a URLSearchParams object
 *
 * @param {Object} val The value to test
 * @returns {boolean} True if value is a URLSearchParams object, otherwise false
 */
function isURLSearchParams(val: any) {
  return val.toString() === '[object URLSearchParams]';
}

/**
 * Determine if a value is a Date
 *
 * @param {Object} val The value to test
 * @returns {boolean} True if value is a Date, otherwise false
 */
function isDate(val: any) {
  return val.toString() === '[object Date]';
}

/**
 * Determine if a value is an Object
 *
 * @param {Object} val The value to test
 * @returns {boolean} True if value is an Object, otherwise false
 */
function isObject(val: any) {
  return val !== null && typeof val === 'object';
}

export function buildURL(
  url: string,
  queries: object | undefined | null,
  params: Record<string, string | number> | undefined | null,
  paramsSerializer:
    | ((params: Record<string, any> | undefined | null) => string)
    | undefined
    | null
) {
  if (params) {
    for (let key of Object.keys(params)) {
      //@ts-ignore
      url = url.replace(`{${key}}`, params[key]);
    }
  }

  /*eslint no-param-reassign:0*/
  if (!queries) return url;

  let serializedParams;
  if (paramsSerializer) {
    serializedParams = paramsSerializer(queries);
  } else if (isURLSearchParams(queries)) {
    serializedParams = queries.toString();
  } else {
    const parts = [];

    for (let key of Object.keys(queries)) {
      //@ts-ignore
      let val = queries[key];
      if (Array.isArray(val)) {
        key = key + '[]';
      } else {
        val = [val];
      }

      for (let v of val) {
        if (isDate(v)) {
          v = v.toISOString();
        } else if (isObject(v)) {
          v = JSON.stringify(v);
        }
        parts.push(encode(key) + '=' + encode(v));
      }
    }

    serializedParams = parts.join('&');
  }

  if (serializedParams) {
    const hashmarkIndex = url.indexOf('#');
    if (hashmarkIndex !== -1) {
      url = url.slice(0, hashmarkIndex);
    }

    url += (url.indexOf('?') === -1 ? '?' : '&') + serializedParams;
  }

  return url;
}

function isAbsoluteURL(url: string) {
  // A URL is considered absolute if it begins with "<scheme>://" or "//" (protocol-relative URL).
  // RFC 3986 defines scheme name as a sequence of characters beginning with a letter and followed
  // by any combination of letters, digits, plus, period, or hyphen.
  return /^([a-z][a-z\d+\-.]*:)?\/\//i.test(url);
}

function combineURLs(baseURL: string, relativeURL: string) {
  return relativeURL
    ? baseURL.replace(/\/+$/, '') + '/' + relativeURL.replace(/^\/+/, '')
    : baseURL;
}

/**
 * Creates a new URL by combining the baseURL with the requestedURL,
 * only when the requestedURL is not already an absolute URL.
 * If the requestURL is absolute, this function returns the requestedURL untouched.
 *
 * @param {string} baseURL The base URL
 * @param {string} requestedURL Absolute or relative URL to combine
 * @returns {string} The combined full path
 */
export function buildFullPath(
  baseURL: string | undefined | null,
  requestedURL: string
) {
  if (baseURL && !isAbsoluteURL(requestedURL)) {
    return combineURLs(baseURL, requestedURL);
  }
  return requestedURL;
}
