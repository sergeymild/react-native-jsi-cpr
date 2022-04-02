export enum JsiMethod {
  get = 'get',
  delete = 'delete',
  head = 'head',
  options = 'options',
  post = 'post',
  put = 'put',
  patch = 'patch',
  purge = 'purge',
  link = 'link',
  unlink = 'unlink',
}

type Query = string | number | boolean;
export type Queries = Record<
  string,
  string | number | boolean | Query[] | undefined | null
>;
export type Headers = Record<string, string>;
export type ParamsSerializer = (params: Queries | undefined | null) => string;

export type ContentType = 'string' | 'json' | 'formUrlEncoded' | 'formData';

type FileDataType =
  | Partial<Record<string, string | number | boolean>>
  | {
      file?: { name: string; path: string };
    };

export type PostParams = Partial<Omit<JsiRequest, 'url' | 'method'>> &
  (WithFile | WithString | WithJson | WithFormUrlEncoded);

export type PatchParams = PostParams;
export type PutParams = PostParams;
export type DeleteParams = Partial<Omit<JsiRequest, 'url' | 'method'>> &
  (WithString | WithJson | WithFormUrlEncoded);

export type WithData = PostParams | DeleteParams | WithFormUrlEncoded;

export type WithFile = {
  dataType: 'formData';
  data?: FileDataType;
};

export type WithString = {
  dataType: 'string';
  data?: string;
};

export type WithFormUrlEncoded = {
  dataType: 'formUrlEncoded';
  data?: Record<
    string,
    string | number | boolean | Array<string | number | boolean>
  >;
};

export type WithJson = {
  dataType: 'json';
  data?: string | object;
};

interface BaseParams {
  headers?: Headers;
  paramsSerializer?: ParamsSerializer;
  errorInterceptor?: (request: JsiError) => Promise<JsiError>;
  timeout?: number;
  baseUrl: string;
}

export interface JsiRequest extends BaseParams {
  method: JsiMethod;
  url: string;
  queries?: Queries;
  params?: Record<string, string | number>;

  requestId?: string;
}

export type LogRequest = (request: Readonly<Partial<JsiRequest>>) => void;
export type LogError = (error: JsiError) => void;
export type LogResponse = (
  request: Readonly<Partial<JsiRequest>>,
  response: Readonly<JsiResponse<any>>
) => void;

export type RequestInterceptor = (
  request: Partial<JsiRequest>
) => Promise<Partial<JsiRequest>>;

export interface JsiDefaultRequest extends BaseParams {
  logRequest?: LogRequest;
  logResponse?: LogResponse;
  logErrorResponse?: LogError;
  skipResponseHeaders?: boolean;
  requestInterceptor?: RequestInterceptor;
}

interface CommonResponse {
  readonly elapsed: number;
  readonly headers?: Record<string, string>;
  readonly requestId: string;
  readonly endpoint: string;
  readonly status: number;
}

export interface JsiError extends CommonResponse {
  readonly type: 'error';
  error: string;
  readonly data?: string;
}

export interface JsiSuccess<T> extends CommonResponse {
  readonly type: 'success';
  readonly data: T;
}

export type JsiResponse<T> = JsiError | JsiSuccess<T>;
