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

type FileDataType = ({path: string, name: string} | {name: string, value: string})[]

export type PostParams = Partial<Omit<JsiRequest, 'url' | 'method'>> &
  (WithFile | WithString | WithJson | WithFormUrlEncoded);

export type PatchParams = PostParams;
export type PutParams = PostParams;
export type DeleteParams = Partial<Omit<JsiRequest, 'url' | 'method'>> &
  (WithString | WithJson | WithFormUrlEncoded);

export type WithData = PostParams | DeleteParams | WithFormUrlEncoded;

export type WithFile = {
  formData?: FileDataType;
};

export type WithString = {
  string?: string;
};

export type WithFormUrlEncoded = {
  formUrlEncoded?: Record<
    string,
    string | number | boolean | Array<string | number | boolean>
    >;
};

export type WithJson = {
  json?: string | object;
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

export type SimpleRequest = Partial<Omit<JsiRequest, 'errorInterceptor' | 'paramsSerializer'> & {data?: WithData | string}>
export type LogRequest = (request: SimpleRequest) => void;
export type LogError = (error: JsiError) => void;
export type LogResponse = (
  request: Readonly<SimpleRequest>,
  response: Readonly<JsiResponse<any>>
) => void;

export type RequestInterceptor = (
  request: SimpleRequest
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
