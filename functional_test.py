import asyncio
import random


def run_command(command, reader, writer, loop):
    ''' Runs a command and reads and parses the response into a Python object.
        command should be a sequence of bytes.
    '''

    async def read_response(reader):
        marker = await reader.read(1)
        if marker == b'+' or marker == b'-':
            response_string = await reader.readline()
            return marker.decode() + response_string[:-2].decode()
        elif marker == b'$':
            response_length = await reader.readline()
            response_length = int(response_length[:-2])
            if response_length == -1:
                return None         # Nullbulk
            print(response_length)
            response_string = await reader.read(response_length)
            await reader.read(2)    # Discard \r\n
            return response_string
        elif marker == b':':
            integer = await reader.readline()
            return int(integer[:-2])
        elif marker == b'*':
            array_length = await reader.readline()
            array_length = int(array_length[:-2])
            ret_array = []
            for _ in range(array_length):
                bstr_length = await reader.readline()
                bstr_length = int(bstr_length[1:-2])
                response_string = await reader.read(bstr_length)
                await reader.read(2)
                ret_array.append(response_string)
            return ret_array

    async def exo_client(command, reader, writer):
        writer.write(command + b'\r\n')
        return await read_response(reader)

    return loop.run_until_complete(exo_client(command, reader, writer))


def random_bytes(length):
    ''' Returns a quoted random sequence of bytes.'''
    seq = list(range(10))
    seq.extend(list(range(11, 32)))
    seq.extend(list(range(35, 92)))
    seq.extend(list(range(93, 256)))
    return bytes(random.choice(seq) for _ in range(length))


def test_get_set(connection, bstr_size):
    reader, writer, loop = connection
    key = random_bytes(bstr_size)
    value = random_bytes(bstr_size)
    response = run_command(b' '.join([b'SET', key, value]), reader, writer,
                           loop)
    assert response == '+OK'
    response = run_command(b' '.join([b'GET', key]), reader, writer, loop)
    print(len(response))
    assert response == value

    value = random_bytes(bstr_size)
    cmd_list = [b'SET', key, value, b'NX']
    response = run_command(b' '.join(cmd_list), reader, writer, loop)
    assert response == None

    key = random_bytes(bstr_size)
    cmd_list = [b'SET', key, value, b'XX']
    response = run_command(b' '.join(cmd_list), reader, writer, loop)
    assert response == None
