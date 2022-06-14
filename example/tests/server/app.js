var express = require('express');
var logger = require('morgan');
const multer = require('multer');

const os = require('os');
const upload = multer({ dest: os.tmpdir() });

var app = express();

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

/** Process POST request */
app.post('/upload', upload.any(), function (req, res, next) {
  console.log('[App.]', req.headers);
  console.log('[App.]', req.body);
  res.json({ data: { file: req.files[0].destination, file2: req.files[1].destination, ...req.body } });
});

app.get('/get/:user/:id', function (req, res, next) {
  console.log('[App.]', req.headers);
  console.log('[App.]', req.body);
  res.json({ data: { url: req.url } });
});

app.patch('/patch', function (req, res, next) {
  console.log('[App.]', req.headers);
  console.log('[App.patch]', req.body);
  res.json({ data: req.body });
});

app.post('/post', function (req, res, next) {
  console.log('[App.]', req.headers);
  console.log('[App.post]', req.body);
  res.json({ data: req.body });
});

app.put('/put', function (req, res, next) {
  console.log('[App.put.headers]', req.headers);
  console.log('[App.put.body]', req.body);
  res.json({ data: req.body });
});

app.delete('/delete', function (req, res, next) {
  console.log('[App.]', req.headers);
  console.log('[App.delete]', req.body);
  res.json({ data: req.body });
});

module.exports = app;
app.listen(3001);
