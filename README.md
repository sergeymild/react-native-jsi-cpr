# react-native-jsi-cpr

React Native JSI wrapper for [cpr](https://github.com/libcpr/cpr) curl HTTP client

###Features
- High performance because everything is written in C++ (even the JS functions have C++ bodies!)
- iOS, Android support

## Installation

```sh
#add to package.json
"react-native-jsi-cpr":"sergeymild/react-native-jsi-cpr#1.3.0"
# after that make yarn install
# and npx pod-install
```

### Import
```typescript
import { JsiHttp } from 'react-native-jsi-cpr';
```

### Usage
```typescript
//create instance
const http = new JsiHttp({
  baseUrl: 'url',
  timeout: 1000,
}, /*isDebug*/ true)

//simple get request
const getResponse = await http.get<{id: string; name: string}>('/user/{id}', {
  params: {id: 20},
  timeout: 500,
})
if (getResponse.type === 'error') {
  console.log(getResponse.error)
} else {
  console.log(getResponse.data)
}

//simple post request, will never throw any errors
const postResponse = await http.post<any>('/post', {
  // also may be 'string' | 'json' | 'formUrlEncoded' | 'formData'
  json: {
    firstName: 'Fred',
    lastName: 'Flintstone'
  }
})
if (postResponse.type === 'error') {
  console.log(postResponse.error)
} else {
  console.log(postResponse.data)
}

//cancel request will throw expception with error status code 80
http.get<any>('/post', {
  requestId: 'uniqueRequestId'
})
http.cancelRequest('uniqueRequestId')
```

### Default config passed on initialization
```typescript
{
  baseURL: 'https://api.example.com',
  // `headers` are base headers will be passed to each request
  headers: {},
  paramsSerializer?: ParamsSerializer;
  errorInterceptor?: (request: JsiError) => Promise<JsiError>;
  // `timeout` specifies the number of milliseconds before the request times out.
  // If the request takes longer than `timeout`, the request will be aborted.
  timeout: 1000, // default is `0` (no timeout)
  timeout?: number;
  logRequest?: LogRequest;
  logResponse?: LogResponse;
  logErrorResponse?: LogError;
  skipResponseHeaders?: boolean;
  requestInterceptor?: RequestInterceptor;
}
```

### Request Config
```typescript

// Base request

{
  // `url` is the server URL that will be used for the request
  url: '/user/{id}';
  // `queries` are the URL query parameters to be sent with the request
  queries?: {orderBy: 'id'};
  // `params` params which will replace placeholders in url
  params?: {id: 12345};
  // `requestId` if not set will be generate automatically, request maybe cancelled with with id
  requestId?: '345';
  // `headers` are custom headers to be sent
  headers: {'X-Requested-With': 'XMLHttpRequest'},
  // `paramsSerializer` is an optional function in charge of serializing `params`
  // (e.g. https://www.npmjs.com/package/qs, http://api.jquery.com/jquery.param/)
  paramsSerializer: (queries) => QS.stringify(queries, {arrayFormat: 'brackets'}),
  // `timeout` specifies the number of milliseconds before the request times out.
  // If the request takes longer than `timeout`, the request will be aborted.
  timeout: 1000, // default is `0` (no timeout)
  // `baseURL` will be prepended to `url` unless `url` is absolute.
  // It can be convenient to set `baseURL` for an instance of axios to pass relative URLs
  // to methods of that instance.
  baseURL: 'https://some-domain.com/api',
  errorInterceptor: (error) => error
}

// Only applicable for request methods 'PUT', 'POST', 'DELETE' and 'PATCH'
{
  // syntax for `json`
  json: {
    firstName: 'Fred'
  }

  // syntax for `string`
  string: 'Fred'

  // syntax for `formUrlEncoded`
  formUrlEncoded: {
    firstName: 'Fred'
  }

  // syntax for `formData`
  formData: [
    // for file
    {name: 'fileName', path: 'absoluteFilePath'},
    {name: 'fileName2', path: 'absoluteFilePath'},
    // for other parameters
    {name: 'fileName', value: 'value'}
  ]
}

```

### Response Schema
```typescript
interface CommonResponse {
  // time which took for request
  readonly elapsed: number;
  // if skipResponseHeaders passed in defaultConfig is true
  // `headers` the HTTP headers that the server responded with
  // All header names are lower cased and can be accessed using the bracket notation.
  // Example: `response.headers['content-type']`
  readonly headers?: Record<string, string>;
  // request id
  readonly requestId: string;
  // full endpoint where request was made
  readonly endpoint: string;
  // `status` is the HTTP status code from the server response
  readonly status: number;
}

export interface JsiError extends CommonResponse {
  readonly type: 'error';
  // `error` is the response that was provided by the server
  error: string;
  // `data` is the response that was provided by the server on error
  readonly data?: string;
}

export interface JsiSuccess<T> extends CommonResponse {
  readonly type: 'success';
  // `data` is the response that was provided by the server
  readonly data: T;
}

export type JsiResponse<T> = JsiError | JsiSuccess<T>;
```

## Tips
1. Depending Another Android Library Containing libcrypto.so, libssl.so.
`react-native-jsi-cpr` packages specified in Android C++ include `libcrypto.so, libssl.so` libraries. If a second library which also includes `libcrypto.so, libssl.so` is added as a dependency, gradle fails with More than one file was found with OS independent path `lib/x86/libcrypto.so` or `lib/x86/libssl.so` error message.

You can fix this error by adding the following block into your build.gradle.
```
android {
    packagingOptions {
        pickFirst 'lib/x86/libcrypto.so'
        pickFirst 'lib/x86/libssl.so'
        pickFirst 'lib/x86_64/libcrypto.so'
        pickFirst 'lib/x86_64/libssl.so'
        pickFirst 'lib/armeabi-v7a/libcrypto.so'
        pickFirst 'lib/armeabi-v7a/libssl.so'
        pickFirst 'lib/arm64-v8a/libcrypto.so'
        pickFirst 'lib/arm64-v8a/libssl.so'
    }
}
```

## Contributing

See the [contributing guide](CONTRIBUTING.md) to learn how to contribute to the repository and the development workflow.

## License

MIT
