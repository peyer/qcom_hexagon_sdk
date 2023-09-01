
def iteritems(d):
    try:
         return d.iteritems()
    except AttributeError:
         return d.items()
