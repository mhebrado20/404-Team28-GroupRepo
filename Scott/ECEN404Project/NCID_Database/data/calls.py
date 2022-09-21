import datetime
import mongoengine
from data.incoming_callers import IncomingCaller
from data.receiving_callers import ReceivingCaller
# from src.data.recordings import Recording


class Call(mongoengine.Document):

    # date call was made; defaults to current time
    date_of_call = mongoengine.DateTimeField(default=datetime.datetime.now)
    # date call was uploaded to database; defaults to current time
    date_of_upload = mongoengine.DateTimeField(default=datetime.datetime.now)
    # original unedited length of the call
    original_length_of_call = mongoengine.StringField(default='0')
    # length of the call after preprocessing the audio and removing silence
    new_length_of_call = mongoengine.StringField(default='0')
    # this is the gridfs embedded document where the actual recording is stored
    call_recording = mongoengine.FileField(collection_name='recordings')
    # this is the name of the file as given by the naming convention specified by user input
    file_name = mongoengine.StringField(default='')

    # incoming_caller_info = mongoengine.EmbeddedDocumentField(IncomingCaller)    to be implemented in 404 when
    # information between database and ML algorithm are sent between each other
    # receiving_caller_info = mongoengine.EmbeddedDocumentField(ReceivingCaller)  ""
    #
    # The Upload_recording function is used to upload a single audio file to the database given a file path and name
    # The default database is a local database hosted on the users machine. This is a variable in case cloud
    # based database is implemented in the future

    def upload_recording(self, file_location: str, file_name: str, database='NCID_local_DB', host='127.0.0.1'):
        with open(file_location, 'rb') as file:
            self.call_recording.put(file, content_type='recording', filename=file_name)
        self.save()

    meta = {
        'db_alias:': 'core',
        'collection': 'calls'
    }
