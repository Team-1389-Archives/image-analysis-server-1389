var net=require('net'),
	child_process=require('child_process');

var processed="";

function stage_processing(){
	var chld=child_process.spawn(__dirname+'/run.sh', [], {
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
		processed=out;
		process.nextTick(stage_processing);
	});
}

stage_processing();


var server=net.createServer(function(socket){
	socket.setEncoding('utf8');
	var times=0;
	function write(){
		if(times===0){
			return;
		}
		times--;
		socket.write(processed, "utf8", write);
	}
	socket.on('data', function(chunk){
		var oldTimes=times;
		for(var i=0;i<chunk.length;i++){
			if(chunk.charAt(i)==='\n'){
				++times;
			}
		}
		if(oldTimes===0){
			write();
		}
	});
});

server.listen(443, '0.0.0.0');
