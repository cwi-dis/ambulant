from dbsupport import ANY, OneOf, FootNote, OS, Renderer, Protocol
import markup

def gen_form(page):
    page.form(action="http://example.com/form")
    page.table(border=1)
    page.tr()
    page.th("Prune the table by making a selection", colspan=2)
    page.tr.close()
    
    page.tr()
    page.td("Operating system:")
    page.td(align="right")
    page.select(name="os", multiple=None)
    for e in OS.entries:
        page.option(e.name)
    page.select.close()
    page.td.close()
    page.tr.close()
    
    page.tr()
    page.td("Operating system version:")
    page.td(align="right")
    page.input(name="os_version")
    page.td.close()
    page.tr.close()
    
    page.tr()
    page.td("Ambulant release:")
    page.td(align="right")
    page.input(name="release")
    page.td.close()
    page.tr.close()
    
    page.tr()
    page.td("Ambulant renderer:")
    page.td(align="right")
    page.select(name="renderer", multiple=None)
    for e in Renderer.entries:
        page.option(e.name)
    page.select.close()
    page.td.close()
    page.tr.close()
    
    page.tr()
    page.td("Access protocol:")
    page.td(align="right")
    page.select(name="protocol", multiple=None)
    for e in Protocol.entries:
        page.option(e.name)
    page.select.close()
    page.td.close()
    page.tr.close()

    page.tr()
    page.td(colspan=2, align="right")
    page.button("Prune Table", type="submit")
    page.td.close()
    page.tr.close()
    page.table.close()
    page.form.close()
    
def gen_html(entries):
    
    page = markup.page()
    page.init(title="Ambulant media support")
    gen_form(page)
    gen_table(page, entries)
    return page
    
def gen_table(page, entries):
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
    page.th('Sample', rowspan=2)
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
            smil = None
        else:
            mimetypes = []
            _genentry(getall(e.format.container, 'mimetype'), e.format_notes)
            _genentry(getall(e.format.container, 'extension'), None)
            _genentry(e.format.audio, None)
            _genentry(e.format.video, None)
            import smilgen
            import database
            if e.proto is database.RTSP:
                prefix = 'smil-rtsp-'
                basename = 'rtsp://media2.cwi.nl/mediasupport/'
            else:
                prefix = 'smil-rel-'
                basename = ''
            if e.renderer is ANY:
                prefix += 'any-'
                renderer_uri = None
            else:
                prefix += str(e.renderer) + '-'
                renderer_uri = e.renderer.renderer_uri
            smil = smilgen.gen_smilfile(prefix, basename, e.format, renderer_uri)
        _genentry(e.supported, e.supported_notes)
        if smil:
            page.td()
            base = smil.split('/')[-1]
            page.a(base, href=smil)
            page.td.close()
        page.tr.close()
    page.table.close()
    
    # Footnotes
    page.dl()
    for e in FootNote.entries:
        if e in footnotes:
            page.dt()
            page.a('[%s]' % e.number, name='footnote_%s' % e.number)
            page.dt.close()
            page.dd()
            page.add(e.text)
            if e.reporter or e.date or e.bug:
                page.i()
                page.add('[Additional info: ')
                if e.reporter: page.add('reported by %s; ' % e.reporter)
                if e.date: page.add('reported on %s; ' % e.date)
                if e.bug:
                    page.add('bug report ')
                    page.a(e.bug, href='https://sourceforge.net/tracker/index.php?func=detail&aid=%s&group_id=91457&atid=597222' % e.bug)
                page.add(']')
                page.i.close()
                
            page.dd.close()
    page.dl.close()

def filter(entries, pattern):
    rv = []
    for e in entries:
        if e == pattern:
            rv.append(e)
    return rv
    
if __name__ == '__main__':
    import database
    import smilgen
#    smilgen.gen_smilfiles('smil-', '', database.MediaFormat.entries)
    page = gen_html(database.E.entries)
    print page
    
    