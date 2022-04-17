#!/bin/python3

import asyncio
import websockets
import json
import base64
import math
from pathlib import Path

#import functools;

class Jsonrpc:
    def __init__(self):
        self.method_map = dict()

    def add_method(self, func):
        self.method_map[func.__name__] = func;
        return func;
        
    def handle(self, msg):
        j={}
        error = None
        try:
            j = json.loads(msg)
        except Exception as e:
            error = repr(e)
            return json.dumps({'id': -1, 'error': error})
        result = { 'id': j.get('id') }
        method = j.get('method');
        if(method in self.method_map):
            func = self.method_map[method];
            params = j.get('params') or []
            try:
                result['result'] = func(*params)
            except (TypeError, IOError) as e:
                error = repr(e)
        else:
            error = "jsonrpc no such func"
        if(error):
            result['error'] = error
        return json.dumps(result);

jrpc = Jsonrpc();

@jrpc.add_method
def readfile(path):
    with open(path, 'rb') as f:
        data = f.read()
        return base64.b64encode(data).decode('ascii')

@jrpc.add_method
def get_dir_size(path, depth):
    glob_strs = ['**/*'] if depth <= 0 else ['/'.join(['*' for _ in range(i+1)]) for i in range(depth)]
    root_directory = Path(path)
    return sum([sum(f.stat().st_size for f in root_directory.glob(s) if f.is_file()) for s in glob_strs])


@jrpc.add_method
def get_folder_list(path, _opt={}):
    def gen_item(f):
        stat = f.stat()
        return {
            'name': f.name,
            'mtime': math.floor(stat.st_mtime)
        }
    opt = get_folder_list.default_opt.copy()
    opt.update(_opt);
    opt_only_dir = opt['only_dir']

    def path_filter(p):
        return p.is_dir() if opt_only_dir else True;

    folder = next(filter(lambda p: p.is_dir(),[Path(p) for p in [path, *opt['fallbacks']]]), None)
    if folder is None:
        return None
    return {
        'folder': str(folder),
        'items': [gen_item(p) for p in folder.glob('*') if path_filter(p)]
    }


get_folder_list.default_opt = {
    'only_dir': True,
    'fallbacks': []
}

async def connect(uri):
    async with websockets.connect(uri) as websocket:
        while True:
            recv = jrpc.handle(await websocket.recv());
            await websocket.send(recv);


if __name__ == '__main__':
    import argparse
    import os
    parser = argparse.ArgumentParser(description='qml localfile helper')
    parser.add_argument("url", metavar='URL', type=str,
                    help='a websocket url')
    args = vars(parser.parse_args())

    if (hasattr(asyncio, 'run')):
        asyncio.run(connect(args['url']))
    else:
        asyncio.get_event_loop().run_until_complete(connect(args['url']))
