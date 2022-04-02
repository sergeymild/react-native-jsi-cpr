import { JsiDefaultRequest, JsiMethod, JsiRequest } from './types';

const commonHeaders = Object.keys(JsiMethod);
export class CurlHelper {
  private request: Partial<JsiRequest & JsiDefaultRequest>;
  constructor(config: Partial<JsiRequest & JsiDefaultRequest>) {
    this.request = config;
  }

  get getHeaders() {
    let headers = this.request.headers ?? {};
    let curlHeaders = '';

    // add any custom headers (defined upon calling methods like .get(), .post(), etc.)
    for (let property in this.request.headers) {
      if (!commonHeaders.includes(property)) {
        //@ts-ignore
        headers[property] = this.request.headers[property];
      }
    }

    for (let property in headers) {
      if ({}.hasOwnProperty.call(headers, property)) {
        let header = `${property}:${headers[property]}`;
        curlHeaders = `${curlHeaders} -H "${header}"`;
      }
    }

    return curlHeaders.trim();
  }

  get getMethod() {
    return `-X ${(this.request.method ?? 'UNKNOWN').toUpperCase()}`;
  }

  get getBody() {
    if (
      typeof this.request.data !== 'undefined' &&
      this.request.data !== '' &&
      this.request.data !== null &&
      (this.request.method ?? 'UNKNOWN').toUpperCase() !== 'GET'
    ) {
      let data =
        typeof this.request.data === 'object' ||
        Object.prototype.toString.call(this.request.data) === '[object Array]'
          ? JSON.stringify(this.request.data)
          : this.request.data;
      return `--data '${data}'`.trim();
    } else {
      return '';
    }
  }

  get getUrl(): string | undefined {
    return this.request.url;
  }

  generateCommand() {
    return `curl ${this.getMethod} "${this.getUrl}" ${this.getHeaders} ${this.getBody}`
      .trim()
      .replace(/\s{2,}/g, ' ');
  }
}
