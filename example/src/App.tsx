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
    baseUrl: 'https://8a6e-62-228-134-206.eu.ngrok.io',
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
          const r = await beceptor.post('upload', {
            formData: [{path: response.assets![0].uri!, name: 'file'}, {path: response.assets![0].uri!, name: 'file2'}, {name: 's', value: 's'}]
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
            json: {
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
            json: {
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
          console.log('[App.BAD_SSL_CHECK]', )
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
              formUrlEncoded: {
                deviceId: 'some',
                email: 'se@y.ru',
              },
            })
            .then((r) => {
              console.log('[App.]', r);
            });
        }}
      >
        <Text>POST(formUrlEncoded)</Text>
      </TouchableOpacity>
      <TouchableOpacity
        style={styles.button}
        onPress={async () => {
          beceptor
            .post('post', {
              json: {
                deviceId: 'some',
                email: 'se@y.ru',
              },
            })
            .then((r) => {
              console.log('[App.]', r);
            });
        }}
      >
        <Text>POST(json)</Text>
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
