from dbsupport import ANY, OneOf, FootNote
import markup

def gen_html(entries):
    
    page = markup.page()
    page.init(title="Ambulant media support")
    page.table(border=1)
    # Table headers
    page.tr()
    page.th('OS', rowspan=2)
    page.th('Version', rowspan=2)
    page.th('Ambulant release', rowspan=2)
    page.th('Renderer', rowspan=2)
    page.th('Protocol', rowspan=2)
    page.th('Media type', colspan=4)
    page.th('Supported', rowspan=2)
    page.tr.close()
    page.tr()
    page.th('Mimetype')
    page.th('Extensions')
    page.th('Audio')
    page.th('Video')
    page.tr.close()
    
    # Table entries
    footnotes = []
    def _genentry(text, notes, colspan=None):
        if not notes:
            page.td(str(text), colspan=colspan)
        else:
            page.td()
            page.add(str(text))
            if type(notes) != type(()):
                notes = (notes,)
            for note in notes:
                page.sup(markup.oneliner.a('[%s]' % note.number, href='#footnote_%s' % note.number))
                if not note in footnotes:
                    footnotes.append(note)
            page.td.close()
    def getall(obj, attrname):
        if isinstance(obj, OneOf):
            rv =[]
            for i in obj.items:
                rv.append(getall(i, attrname))
            return ', '.join(rv)
        return str(getattr(obj, attrname))
    for e in entries:
        page.tr()
        _genentry(e.os, e.os_notes)
        _genentry(e.osversion, e.osversion_notes)
        _genentry(e.release, e.release_notes)
        _genentry(e.renderer, e.renderer_notes)
        _genentry(e.proto, e.proto_notes)
        if e.format is ANY:
            _genentry(ANY, e.format_notes, colspan=4)
        else:
            mimetypes = []
            _genentry(getall(e.format.container, 'mimetype'), e.format_notes)
            _genentry(getall(e.format.container, 'extension'), None)
            _genentry(e.format.audio, None)
            _genentry(e.format.video, None)
        _genentry(e.supported, e.supported_notes)
        page.tr.close()
    page.table.close()
    
    # Footnotes
    page.dl()
    for e in FootNote.entries:
        if e in footnotes:
            page.dt()
            page.a('[%s]' % e.number, name='footnote_%s' % e.number)
            page.dt.close()
            page.dd(e.text)
    page.dl.close()
    
    print page

def filter(entries, pattern):
    rv = []
    for e in entries:
        if e == pattern:
            rv.append(e)
    return rv
    
if __name__ == '__main__':
    import database
    f = database.Q(proto="http")
    gen_html(filter(database.E.entries, f))
    