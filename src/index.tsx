import {
  ContentType,
  DeleteParams,
  Headers,
  JsiDefaultRequest,
  JsiError,
  JsiMethod,
  JsiRequest,
  JsiResponse,
  PatchParams,
  PostParams,
  PutParams,
  WithData,
} from './types';
import { buildURL } from './buildURL';
import { CurlHelper } from './CurlHelper';

export {JsiError, JsiRequest}

import { NativeModules, Platform } from 'react-native';
import { LogError, LogRequest, LogResponse } from "react-native-jsi-cpr/src/types";
const LINKING_ERROR =
  `The package 'react-native-jsi-websockets' doesn't seem to be linked. Make sure: \n\n` +
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

const prepareDataType = (
  contentType?: ContentType,
  config?: Partial<Omit<JsiRequest, 'url' | 'method'>> & WithData
): any => {
  if (!config) return undefined;
  if (!config.data) return undefined;
  let data: any = config.data;
  if (contentType === 'json' && typeof config.data === 'object') {
    data = JSON.stringify(config.data);
  }

  if (config.dataType === 'formUrlEncoded') {
    data = buildURL('', data, config.params, null).replace('?', '');
  }

  if (config.dataType === 'string' && typeof data !== 'string') {
    throw new Error('data must be a string');
  }

  return data;
};

const prepareContentType = (
  config?: Partial<Omit<JsiRequest, 'url' | 'method'>> & WithData
): ContentType | undefined => {
  if (!config) return 'json';
  return config.dataType;
};

export const defaultLogRequest: LogRequest = (params: Readonly<Partial<JsiRequest>>) => {
  console.log('[REQUEST]', new CurlHelper(params).generateCommand());
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
    let contentType = prepareContentType(config);
    let data = prepareDataType(contentType, config);
    return this.request<T>({
      ...config,
      data: data,
      method: JsiMethod.delete,
      dataType: contentType,
      url: url,
      headers: getHeaders(config),
    });
  }

  post<T = any>(url: string, config: PostParams): Promise<JsiResponse<T>> {
    let contentType = prepareContentType(config);
    let data = prepareDataType(contentType, config);
    return this.request<T>({
      ...config,
      data: data,
      method: JsiMethod.post,
      dataType: contentType,
      url: url,
      headers: getHeaders(config),
    });
  }

  put<T = any>(url: string, config: PutParams): Promise<JsiResponse<T>> {
    let contentType = prepareContentType(config);
    let data = prepareDataType(contentType, config);
    return this.request<T>({
      ...config,
      data: data,
      method: JsiMethod.put,
      dataType: contentType,
      url: url,
      headers: getHeaders(config),
    });
  }

  patch<T = any>(url: string, config: PatchParams): Promise<JsiResponse<T>> {
    let contentType = prepareContentType(config);
    let data = prepareDataType(contentType, config);
    return this.request<T>({
      ...config,
      data: data,
      method: JsiMethod.patch,
      dataType: contentType,
      url: url,
      headers: getHeaders(config),
    });
  }

  cancelRequest = (requestId: string) => {
    //@ts-ignore
    global.jsiHttp.httpCancelRequest(requestId);
  };
}
