var express=require('express'),
	app=express(),
	server=require('http').createServer(app),
	io=require('socket.io').listen(server),
	fs=require('fs'),
	child_process=require('child_process')
	uuid=require('node-uuid');

var TESTSUITE_PATH=__dirname+'/testsuite.json';
var testsuite=JSON.parse(fs.readFileSync(
	TESTSUITE_PATH, 
	'utf8'//Apparently this isn't an options object like the documentation purports
));

var TEST_IMAGES=__dirname+'/test-images';
try{
	fs.mkdirSync(__dirname+'/test-images');
}catch(e){/*It already exists.*/}

server.listen(process.env.PORT||9000, process.env.IP||'0.0.0.0');

app.use(express.logger());
app.use(express.json());
app.use(express.urlencoded());
app.use(express.static(__dirname + '/static'));
app.use('/test-images/', express.static(TEST_IMAGES));

app.post('/upload', function(req, res){
	var id=uuid.v4();
	res.set('Cache-Control', 'no-cache');
	var src='test-images/'+id+'.jpg';
	var data=new Buffer(req.body.data, "base64");
	fs.writeFile(TEST_IMAGES+'/'+id+'.jpg', data, function(err){
		if(err){
			throw err;
		}
		res.set('Content-Type', 'text/plain');
		res.status(200).send(src);
	});
});

app.get('/testsuite.json', function(req, res){
	res.set('Cache-Control', 'no-cache');
	res.set('Content-Type', 'application/json; charset=utf-8');
	res.status(200).send(JSON.stringify(testsuite));
});

app.post('/testsuite.json', function(req, res){
	res.set('Cache-Control', 'no-cache');
	testsuite=req.body;
	fs.writeFile(TESTSUITE_PATH, JSON.stringify(testsuite), function(err){
		if(err){
			throw err;
		}
		res.send(204);
	});
});

app.post('/run-test', function(req, res){
	res.set('Cache-Control', 'no-cache');
	res.set('Content-Type', 'text-plain; charset=utf-8');
	var chld=child_process.spawn(__dirname+'/run-test.sh', [req.body.img], {
		cwd:__dirname,
		stdio: [null, 'pipe', process.stderr]
	});
	var out="";
	chld.stdout.on('data', function(data){
		out+=data;
	});
	chld.on('close', function(code){
		if(code!==0){
			console.log('error running test with code '+code);
		}
		res.send(out);
	});
});