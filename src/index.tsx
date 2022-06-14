import {
  DeleteParams,
  Headers,
  JsiDefaultRequest,
  JsiError,
  JsiMethod,
  JsiRequest,
  JsiResponse, LogError, LogRequest, LogResponse,
  PatchParams,
  PostParams,
  PutParams,
  SimpleRequest,
  WithData,
} from './types';
import { buildURL } from './buildURL';
import { CurlHelper } from './CurlHelper';

import { NativeModules, Platform } from 'react-native';

export {JsiError, JsiRequest, JsiResponse}

const LINKING_ERROR =
  `The package 'react-native-jsi-cpr' doesn't seem to be linked. Make sure: \n\n` +
  Platform.select({ ios: "- You have run 'pod install'\n", default: '' }) +
  '- You rebuilt the app after installing the package\n' +
  '- You are not using Expo managed workflow\n';

const JsiCpr = NativeModules.JsiCpr
  ? NativeModules.JsiCpr
  : new Proxy(
    {},
    {
      get() {
        throw new Error(LINKING_ERROR);
      },
    }
  );

JsiCpr.install();




let incr = 0;

const getHeaders = (config: any | undefined | null) =>
  config?.headers ? config.headers : {};

const prepareEndpoint = (
  defaultConfig: JsiDefaultRequest,
  params: Partial<JsiRequest> & { url: string }
): string => {
  let url = params.url;
  if (url.length > 0) {
    url = url.startsWith('/') ? url : `/${url}`;
  }
  return buildURL(
    url,
    params.queries,
    params.params,
    params.paramsSerializer ?? defaultConfig.paramsSerializer
  );
};

const prepareHeaders = (
  defaultConfig: JsiDefaultRequest,
  params: Partial<JsiRequest> & { url: string }
): Headers => {
  return defaultConfig.headers || params.headers
    ? {
      ...defaultConfig.headers,
      ...params.headers,
    }
    : {};
};

const applyRequestInterceptor = async (
  defaultConfig: JsiDefaultRequest,
  params: Partial<JsiRequest>
): Promise<Partial<JsiRequest>> => {
  if (defaultConfig.requestInterceptor) {
    return defaultConfig.requestInterceptor?.(params);
  }
  return params;
};

const prepareRequestParams = (
  defaultConfig: JsiDefaultRequest,
  params: Partial<JsiRequest> & { url: string }
): Partial<JsiRequest> => {
  return {
    ...defaultConfig,
    ...params,
    method: params.method!.toUpperCase() as JsiMethod,
    headers: prepareHeaders(defaultConfig, params),

    baseUrl: params.baseUrl ?? defaultConfig.baseUrl,
    url: prepareEndpoint(defaultConfig, params),
  };
};

const prepareResponse = (response: any) => {
  if ('data' in response && typeof response.data === 'string') {
    try {
      response.data = JSON.parse(response.data);
    } catch {}
  }
  return response;
};

const processError = async (
  defaultConfig: JsiDefaultRequest,
  params: Partial<JsiRequest>,
  error: JsiError
): Promise<JsiError> => {
  const interceptor = params.errorInterceptor ?? defaultConfig.errorInterceptor;
  if (interceptor) return interceptor(error);
  return error;
};

const prepareDataType = (config?: Partial<Omit<JsiRequest, 'url' | 'method'>> & WithData) => {
  if (!config) return;
  if ('json' in config) {
    // @ts-ignore
    config.data = {json: JSON.stringify(config.json)}
  }

  if ('string' in config) {
    // @ts-ignore
    config.data = {string: config.string}
  }

  if ('formData' in config) {
    // @ts-ignore
    config.data = {formData: config.formData}
  }

  if ('formUrlEncoded' in config) {
    // @ts-ignore
    config.data = {
      formUrlEncoded: buildURL('', config.formUrlEncoded, config.params, null).replace('?', '')
    }
  }
};

export const defaultLogRequest: LogRequest = (request: SimpleRequest) => {
  if ('data' in request && request.data) {
    //@ts-ignore
    request.data = request.data?.json ?? request.data?.formData ?? request.data?.string
  }
  console.log('[REQUEST]', new CurlHelper(request).generateCommand());
};

export const defaultLogResponse: LogResponse = (
  _request: Readonly<Partial<JsiRequest>>,
  response: Readonly<JsiResponse<any>>
) => {
  console.log(`[RESPONSE]`, JSON.stringify(response));
};

export const defaultLogErrorResponse: LogError = (error: JsiError) => {
  console.log(`[ERROR]`, JSON.stringify(error));
};

const processResponse = async (
  defaultConfig: JsiDefaultRequest,
  params: Readonly<Partial<JsiRequest>>,
  response: any,
  error: any,
  resolve: (response: any) => void
) => {
  try {
    if (defaultConfig.logRequest) {
      defaultConfig.logRequest?.({
        ...params,
        url: (params.baseUrl ?? defaultConfig.baseUrl) + params.url,
      });
    }
    if (error) {
      const processedError = await processError(defaultConfig, params, error);
      if (defaultConfig.logErrorResponse) {
        defaultConfig.logErrorResponse?.({ ...processedError });
      }
      if (defaultConfig.logResponse) {
        defaultConfig.logResponse({ ...params }, { ...processedError });
      }
      resolve(processedError);
    } else {
      response = prepareResponse(response);
      if (defaultConfig.logResponse) {
        defaultConfig.logResponse({ ...params }, { ...response });
      }
      resolve(response);
    }
  } catch (e: any) {
    resolve({
      data: e,
      error: 'UNKNOWN_ERROR',
      type: 'error',
      requestId: params.requestId ?? '-1',
      status: response?.status ?? error?.status ?? -1,
      elapsed: response?.elapsed ?? error?.elapsed ?? -1,
      endpoint:
        response?.endpoint ??
        error?.endpoint ??
        `${params.baseUrl}/${params.url}`,
      headers: getHeaders(response ?? error),
    });
  }
};

export class JsiHttp {
  constructor(private defaultConfig: JsiDefaultRequest, isDebug: boolean) {
    if (!this.defaultConfig.logErrorResponse && isDebug) {
      this.defaultConfig.logErrorResponse = defaultLogErrorResponse;
    }

    if (!this.defaultConfig.logRequest && isDebug) {
      this.defaultConfig.logRequest = defaultLogRequest;
    }

    if (!this.defaultConfig.logResponse && isDebug) {
      this.defaultConfig.logResponse = defaultLogResponse;
    }
  }

  async request<T = any>(
    params: Partial<JsiRequest> & { url: string } & Partial<WithData>
  ): Promise<JsiResponse<T>> {
    return new Promise(async (resolve) => {
      let requestParams = prepareRequestParams(this.defaultConfig, params);
      requestParams = await applyRequestInterceptor(
        this.defaultConfig,
        requestParams
      );
      requestParams = await applyRequestInterceptor(
        this.defaultConfig,
        requestParams
      );
      if (requestParams.requestId === undefined)
        requestParams.requestId = (incr++).toString();
      //@ts-ignore
      global.jsiHttp.makeHttpRequest(
        requestParams,
        (response: any, error: any) => {
          processResponse(
            this.defaultConfig,
            requestParams,
            response,
            error,
            resolve
          );
        }
      );
    });
  }

  get<T = any>(
    url: string,
    config?: Partial<Omit<JsiRequest, 'url' | 'method'>>
  ): Promise<JsiResponse<T>> {
    return this.request<T>({
      ...config,
      method: JsiMethod.get,
      url: url,
    });
  }

  delete<T = any>(url: string, config?: DeleteParams): Promise<JsiResponse<T>> {
    prepareDataType(config)
    return this.request<T>({
      ...config,
      method: JsiMethod.delete,
      url: url,
      headers: getHeaders(config),
    });
  }

  post<T = any>(url: string, config: PostParams): Promise<JsiResponse<T>> {
    prepareDataType(config)
    return this.request<T>({
      ...config,
      method: JsiMethod.post,
      url: url,
      headers: getHeaders(config),
    });
  }

  put<T = any>(url: string, config: PutParams): Promise<JsiResponse<T>> {
    prepareDataType(config)
    return this.request<T>({
      ...config,
      method: JsiMethod.put,
      url: url,
      headers: getHeaders(config),
    });
  }

  patch<T = any>(url: string, config: PatchParams): Promise<JsiResponse<T>> {
    prepareDataType(config)
    return this.request<T>({
      ...config,
      method: JsiMethod.patch,
      url: url,
      headers: getHeaders(config),
    });
  }

  cancelRequest = (requestId: string) => {
    //@ts-ignore
    global.jsiHttp.httpCancelRequest(requestId);
  };
}
