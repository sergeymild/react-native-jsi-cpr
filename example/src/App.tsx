import * as React from 'react';
import { launchImageLibrary } from 'react-native-image-picker';

import { StyleSheet, Text, TouchableOpacity, View } from 'react-native';

import { JsiHttp } from 'react-native-jsi-cpr';


const httpbin = new JsiHttp(
  {
    skipResponseHeaders: false,
    baseUrl: 'https://www.httpbin.org',
  },
  true
);

const beceptor = new JsiHttp(
  {
    skipResponseHeaders: false,
    baseUrl: 'https://b2d2-212-50-119-205.ngrok.io',
  },
  true
);

export default function App() {

  return (
    <View style={styles.container}>

      <TouchableOpacity
        style={styles.button}
        onPress={async () => {
          const response = await launchImageLibrary({ mediaType: 'photo' });
          console.log('[App.]', response.assets[0].uri);

          const r = await beceptor.post('upload', {
            dataType: 'formData',
            data: {
              file: { name: 'file', path: response.assets![0].uri! },
              pas: 'ds',
            },
          });
          console.log('[App.upload]', JSON.stringify(r));
        }}
      >
        <Text>FILE(formData)</Text>
      </TouchableOpacity>

      <TouchableOpacity
        style={styles.button}
        onPress={async () => {
          const r = await beceptor.patch('patch', {
            dataType: 'json',
            data: {
              str: 'one',
              num: 2,
              bool: true,
            },
          });
          console.log('[App.upload]', JSON.stringify(r));
        }}
      >
        <Text>PATCH(json)</Text>
      </TouchableOpacity>

      <TouchableOpacity
        style={styles.button}
        onPress={async () => {
          const r = await beceptor.delete('delete', {
            dataType: 'json',
            data: {
              str: 'one',
              num: 2,
              bool: true,
            },
          });
          console.log('[App.upload]', JSON.stringify(r));
        }}
      >
        <Text>DELETE(json)</Text>
      </TouchableOpacity>

      <TouchableOpacity
        style={styles.button}
        onPress={async () => {
          const r = await beceptor.get('get/{user}/{id}', {
            params: { user: 'userPath', id: 'userId' },
            queries: {
              one: 'one',
              two: 2,
              three: true,
              four: ['one', 2, true],
            },
          });
          console.log('[App.upload]', JSON.stringify(r));
        }}
      >
        <Text>GET(params)</Text>
      </TouchableOpacity>

      <TouchableOpacity
        style={styles.button}
        onPress={() => {
          httpbin
            .get('', {
              baseUrl: 'https://expired.badssl.com',
              timeout: 3000,
            })
            .then((r) => console.log('[App.response]', r));
        }}
      >
        <Text>BAD_SSL_CHECK</Text>
      </TouchableOpacity>

      <TouchableOpacity
        style={styles.button}
        onPress={async () => {
          beceptor
            .post('post', {
              dataType: 'formUrlEncoded',
              data: {
                deviceId: 'some',
                email: 'se@y.ru',
              },
            })
            .then((r) => {
              console.log('[App.]', r);
            });
        }}
      >
        <Text>post</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    paddingTop: 56,
    flex: 1,
    flexDirection: 'row',
    flexWrap: 'wrap',
    paddingHorizontal: 16,
  },
  button: {
    height: 32,
    backgroundColor: 'gray',
    justifyContent: 'center',
    paddingHorizontal: 8,
    borderRadius: 4,
    marginEnd: 16,
    marginBottom: 16,
  },
});
