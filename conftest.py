import pytest
import asyncio
import subprocess


@pytest.fixture(scope='module')
def connection(request):
    loop = asyncio.get_event_loop()
    reader, writer = loop.run_until_complete(asyncio.open_connection('127.0.0.1',
        15000, loop=loop))
    def fin():
        writer.close()  # Close the socket
        loop.close()
    request.addfinalizer(fin)
    return reader, writer, loop


@pytest.fixture(params=[32, 128, 2048])
def bstr_size(request):
    return request.param


@pytest.fixture()
def run_server(request):    # Probably won't work
    proc = subprocess.Popen(['./exoredis', 'ftest.erdb'])
    def fin():
        proc.send_signal(subprocess.SIGINT)
    request.addfinalizer(fin)
