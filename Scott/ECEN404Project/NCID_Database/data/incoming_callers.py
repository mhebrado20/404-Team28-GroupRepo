import mongoengine


class IncomingCaller(mongoengine.EmbeddedDocument):
    # voice signature of incoming caller
    voice_signature_id = mongoengine.EmbeddedDocumentField()
    voice_signature_name = mongoengine.StringField(default='John Doe')
    # number of total calls made
    number_of_calls = mongoengine.IntField()
    # an identifier corresponding to the voice signature for easier look up
    # will be a mongoengine embedded document
    meta = {
        'db_alias:': 'core',
        'collection': 'incoming callers'
    }
