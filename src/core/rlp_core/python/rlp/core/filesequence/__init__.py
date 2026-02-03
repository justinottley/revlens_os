
import os
import re

import logging
import itertools

from rlp.core import rlp_logging
import rlp.core.framesequence.cmd


# regex for matching numerical characters plus letter inserts
DIGITS_INSERTS_RE = re.compile(r'((\d+)([A-Za-z]*))')

# regex for matching numerical characters plus the sentinel ".None."
DIGITS_NONE_RE = re.compile(r'((\d+)|(None))')

# for matching a version number.
# Workflow specific rules:
#  - version number prefixed with the letter "v"
#  - padding to 3 digits
#  - version number appears right before the extension
VERSION_RE = re.compile('_v(\d{3})\.')
VERSION_PADDING = 3

# strict versioning is going to raise an exception
# if versioning does not conform to studio naming convention
STRICT_VERSIONING = False

rlogger = logging.getLogger('rlp.{}'.format(__name__))


class ItemVersionError(Exception): pass
class SequenceError(Exception): pass
class ItemVersionError(SequenceError): pass
class NoSequenceError(SequenceError): pass
class SignatureHintError(SequenceError): pass

class ItemVersion(object):
    
    def __init__(self, ver_num):
        
        self.__version_num = ver_num
        
    def __str__(self):
        return 'v{version}'.format(version=str(self.__version_num).zfill(
            VERSION_PADDING))
        
    @property
    def num(self):
        return self.__version_num
        
    @property
    def padded_num(self):
        return str(self.__version_num).zfill(VERSION_PADDING)
        
    @property
    def string(self):
        return str(self)
        
    def __next__(self):
        return ItemVersion(self.__version_num + 1)
        
    def prev(self):
        
        if self.__version_num == 1:
            raise ItemVersionError('no previous version')
            
        return ItemVersion(self.__version_num - 1)


class ItemSignature(list):
    
    def __init__(self, non_digits, sig_list):
        
        self.__non_digits = non_digits
        list.__init__(self, sig_list)
        
    @property
    def non_digits(self):
        return self.__non_digits
        
        
    def __eq__(self, item_sig):
        
        result = True
        
        if self.non_digits != item_sig.non_digits:
            result = False
            
        elif len(self) != len(item_sig):
            result = False
            
        else:
            for (left, right) in zip(self, item_sig):
                
                if left['digit_idx'] != right['digit_idx']:
                    result = False
                    break
                
        return result


class Item(object):
    
    def __init__(self, raw_item):
        
        self._item = raw_item
        
        dirname, filename = os.path.split(raw_item)
        self._raw_item = filename
        self.__dirname = dirname
        
        self.__version = self._get_version(raw_item)
        self.__version_idx = None
        try:
            self.__version_idx = self._raw_item.index(str(self.__version))
            
        except:
            pass
        
        
        self.__signature = self._get_signature(self._raw_item)
        self.__inserts = []
        
        # a pointer back to the sequence for item
        self.__sequence = None
        
        # a saved iteration entry (based on sequence iteration index)
        # in other words, the varying digit string in the sequence
        self.__iter_entry = None
        
        # the size of the padding for the iter entry
        self.__padding = None
        
        
    def _get_version(self, raw_item):
        
        version_result = re.search(VERSION_RE, raw_item)
        
        if version_result:
            return ItemVersion(int(version_result.group(1)))
        
        
    def _get_signature(self, raw_item):
        
        # digits_result = re.findall(DIGITS_INSERTS_RE, raw_item)
        digits_result = re.findall(DIGITS_NONE_RE, raw_item)
        
        
        curr_idx = 0
        digit_idx_offset = 0
        non_digits = ''
        
        substr_item = raw_item
        
        sig_list = []
        
        
        for entry in digits_result:
            
            item_idx = substr_item.index(entry[0])    
            
            start_idx = curr_idx + item_idx
            end_idx = item_idx + len(entry[0])
            
            non_digits += raw_item[curr_idx:start_idx]
            
            curr_idx += end_idx
            digit_idx = start_idx - digit_idx_offset
            
            substr_item = raw_item[curr_idx:]
            
            sig_list.append(dict(start_idx=start_idx,
                                 digit_idx=digit_idx,
                                 entry=entry[0],
                                 digits=entry[1],
                                 insert=entry[2]))
            
            digit_idx_offset += len(entry[2])
            
        
        non_digits += raw_item[curr_idx:]
        
        
        return ItemSignature(non_digits, sig_list)
        
        
    def __repr__(self):
        return "<Item {name}>".format(name=self._raw_item)
        
    def __str__(self):
        return self.filename
        
    def _get_iter_entry(self):
        
        for entry in self.signature:
            if entry['digit_idx'] == self.sequence.iter_idx:
                return entry['entry']
        
        
    @property
    def filename(self):
        return self._raw_item
        
    @property
    def dirname(self):
        return self.__dirname
        
    @property
    def fullpath(self):
        return '/'.join([self.dirname, self.filename])
        
    @property
    def signature(self):
        return self.__signature
        
    @property
    def version(self):
        return self.__version
        
    @property
    def padding(self):
        
        if self.__padding is None:
            
            for entry in self.signature:
                if entry['digit_idx'] == self.sequence.iter_idx:
                    self.__padding = len(entry['digits'])
        
        
        return self.__padding
        
    @property
    def f(self):
        
        if self.__iter_entry is None:
            self.__iter_entry = self._get_iter_entry()
            
        return self.__iter_entry
        
        
    @property
    def template(self):
        '''
        A format string containing substitution parameters for:
        - iteration string as {f}
        - version as {version} only if Sequence.do_templated_version is True
        '''
        
        if not self.sequence:
            raise SequenceError('cannot produce template - item is not part of a sequence')
            
        template = ''
        curr_idx = 0
        
        for sigentry in self.signature:
            
            start_idx = sigentry['start_idx']
            version_idx = self.__version_idx
            
            if version_idx is None and STRICT_VERSIONING:
                raise ItemVersionError('invalid format for version: {raw_item}'.format(
                    raw_item=self._raw_item))
                
            end_idx = start_idx + len(sigentry['entry'])
            
            if sigentry['digit_idx'] == self.sequence.iter_idx:
                template += self._raw_item[curr_idx:start_idx]
                template += '{f}'
            
            elif version_idx is not None and (start_idx == (version_idx + 1)):
                template += self._raw_item[curr_idx:version_idx]
                
                # use a template string for {version} only if the sequence
                # has that flag set, otherwise leave the version as-is in 
                # the template
                #
                if self.sequence.do_templated_version:
                    template += '{version}'
                    
                else:
                    template += str(self.version)
                
            else:
                
                template += self._raw_item[curr_idx:start_idx]
                template += self._raw_item[start_idx:end_idx]
                
            
            curr_idx = end_idx
            
            
        template += self._raw_item[curr_idx:]
        
        return template
        
        
    @property
    def prefix(self):
        return self.template.format(f=self.f, version=self.version).split('.')[0]
        
    @property
    def extension(self):
        return '.'.join(self.template.split('.')[1:])
        
    @property
    def prefix_template(self):
        return self.template.split('.')[0]
        
        
    def _get_sequence(self):
        return self.__sequence
        
    def _set_sequence(self, sequence):
        self.__sequence = sequence
        
        
    sequence = property(_get_sequence, _set_sequence)
    
    
    
class SignatureHintBase(object):
    '''
    A signature hint is an encapuslation of a predefined file naming convention
    to help identify the iteration index of a sequence.
    A signature hint is used to possibly identify a sequence using a
    single file, if the filename matches the hint, and to disambiguate a
    collection of files in a directory that may appear to have multiple
    sequencing possibilities.
    '''
    
    @property
    def name(self):
        return self.__class__.__name__.replace('SignatureHint', '')
        
    def identify(self, raw_item):
        raise NotImplementedError
        
        
class FrameSignatureHint(SignatureHintBase):
    '''
    A hint for a frame of the form prefix.frame.ext. Or prefix.{f}.ext.
    Version optional.
    '''
    
    SIG_RE = re.compile('(.+)\.(\d+)\.(\w+)')
    SIG_NONE_RE = re.compile('(.+)\.(None)\.(\w+)') # None as the frame number
    
    def identify(self, raw_item):
        
        seq_idx = -1
        
        for regex in [FrameSignatureHint.SIG_RE, FrameSignatureHint.SIG_NONE_RE]:
            result = re.match(regex, raw_item)
            if result:
                seq_idx = len(result.group(1)) + 1
                break
                
            
        return seq_idx
        
        
class NoDotsSignatureHint(SignatureHintBase):
    '''
    A hint for a frame number at the end of a filename
    '''
    
    SIG_RE = re.compile('(.+)_(\d+)\.(\w+)')
    
    def identify(self, raw_item):
        
        seq_idx = -1
        
        result = re.match(NoDotsSignatureHint.SIG_RE, raw_item)
        if result:
            seq_idx = len(result.group(1)) + 1
            
            
        return seq_idx
        
        
class UserSignatureHint(SignatureHintBase):
    '''
    Provides a way for users to register a signature hint using a regular
    expression. The regular expression must have a group named "frame" for the
    hint to identify where in the filename the iteration number is.
    '''
    
    FRAME_GROUP = "?P<frame>"
    
    def __init__(self, hint_name, regex, offset=0):
        
        self._name = hint_name
        self._logger = logging.getLogger('rlp.{ns}.{cls}.{name}'.format(
            ns=self.__class__.__module__,
            cls=self.__class__.__name__,
            name=self._name))
        
        
        # make sure the regex is compatible with the hint
        if self.FRAME_GROUP not in regex:
            raise SignatureHintError("incompatible regex, expect {fg}".format(
                fg=self.FRAME_GROUP))
            
        self.regex = regex
        self.offset = offset
        
        
    @property
    def name(self):
        return self._name
        
        
    def identify(self, raw_item):
        
        self._logger.verbose('attempting to match {item} with {regex}'.format(
            item=raw_item, regex=self.regex))
        
        seq_idx = -1
        result = re.match(self.regex, raw_item)
        if result:
            
            self._logger.verbose('hint match result: {groups}'.format(
                groups=result.groups()))
            
            # iterate on the groups and figure out what the seq_idx is(index
            # i.e., index of the start of the iteration number in the string
            
            frame_result = result.group('frame')
            seq_idx = self.offset
            for group_idx in range(len(result.groups())):
                
                if result.group(group_idx + 1) != frame_result:
                    seq_idx += len(result.group(group_idx + 1))
                    
                else:
                    break
                    
            
            
        self._logger.verbose('result seq_idx: {seq_idx}'.format(seq_idx=seq_idx))
        
        return seq_idx
        
        
class Sequence(object):
    
    _HINTS = [FrameSignatureHint()]
    
    def __init__(self, items=None):
        
        self.do_templated_version = False
        
        self.__padding = None
        
        # The iteration index is the index of the start of the sequence's item
        # digit string that is varying for this sequence
        self.__iter_idx = None
        
        # a map of the digit strings that are varying in this sequence to
        # the items
        self.__iter_digit_map = {}
        
        # a cached sorted list of iter items
        self.__sorted_iter_list = None
        
        
        if items:
            for item in items:
                self.append(item)
        
        
        
    def __repr__(self):
        
        
        if len(self) > 0:
            
            template_str = self[0].template
            if self[0].version:
                template_str = template_str.format(f='{f}', version=self[0].version)
                
            return "<Sequence {template} {frange}>".format(
                template=template_str, frange=self.frange)
            
        else:
            return object.__repr__(self)
            
            
    def __contains__(self, item):
        return item in [str(value) for value in list(self.__iter_digit_map.values())]
        
        
    def __iter__(self):
        
        for entry in self._sorted_iter_items:
            yield self.__iter_digit_map[entry]
        
        
    def __getitem__(self, idx):
        
        entry = self._sorted_iter_items[idx]
        return self.__iter_digit_map[entry]
        
    def __len__(self):
        return len(self._sorted_iter_items)
        
    def _get_compressed_frange(self):
        '''
        Calculate a compressed framerange string for the sequence.
        E.g., "1-20", "1-10,12,13-20"
        
        Inserts are added solo:
        "1-20, 20a, 21-30"
        '''
        
        # initial check - Sequence may not ready to get a compressed frange
        if len(self._sorted_iter_items) == 1 and self._sorted_iter_items[0] == None:
            return ''
        
        return rlp.core.framesequence.cmd.get_frame_description(self._sorted_iter_items)
        
        
    def _reset(self):
        self.__sorted_iter_list = sorted(self.__iter_digit_map)
        
        
    def clear(self):
        
        self.__padding = None
        self.__iter_idx = None
        self.__iter_digit_map = {}
        self.__sorted_iter_list = None
        
        
    @property
    def _sorted_iter_items(self):
        
        if self.__sorted_iter_list is None:
            self.__sorted_iter_list = sorted(self.__iter_digit_map)
        
        return self.__sorted_iter_list
        
        
    @property
    def iter_idx(self):
        return self.__iter_idx
        
        
    @property
    def frange(self):
        return self._get_compressed_frange()
        
        
    @property
    def fstart(self):
        return self[0].f
        
        
    @property
    def fend(self):
        return self[-1].f
        
        
    @property
    def template(self):
        return self[0].template
        
        
    @property
    def prefix(self):
        return self[0].prefix
        
        
    @property
    def template_fullpath(self):
        '''
        A representation that returns the full path to the sequence
        plus its template string
        '''
        
        return os.path.join(
            os.path.realpath(self[0].dirname),
            self[0].template)
        
        
    @property
    def padding(self):
        return self[0].padding
        
        
    @property
    def version(self):
        return self[0].version
        
    @property
    def dirname(self):
        return self[0].dirname
        
        
    def match(self, item):
        '''
        Return whether the input item matches the sequence's item signature and 
        whether the item matches the sequence's sequence iteration signature
        '''
        
        def _get_iter_index(left_item, right_item):
            '''
            get the iteration index between two items - the iteration index
            is the index of the start of a digit entry that is varying
            between the two items
            '''
            
            # count the number of digit entries that are varying. If more than
            # one set of digits is varying, there is no match
            vary_count = 0
            
            iter_idx = -1
            
            for (left_sig, right_sig) in zip(
                left_item.signature, right_item.signature):
                
                if (left_sig['entry'] != right_sig['entry']):
                    
                    assert(left_sig['digit_idx'] == right_sig['digit_idx'])
                    
                    iter_idx = left_sig['digit_idx']
                    vary_count += 1
                    
                    
            if vary_count > 1:
                iter_idx = -1
                
                
            return iter_idx
            
            
        def _get_iter_entry(item, iter_idx):
            
            for sig_entry in item.signature:
                if sig_entry['digit_idx'] == iter_idx:
                    return sig_entry['entry']
                    
            
            
        result = False
        item_iter_result = None
        
        if len(self) == 0:
            
            # attempt to identify the sequence using a hint
            for hint in self._HINTS:
                
                hint_result = hint.identify(item._raw_item)
                
                if hint_result >= 0:
                    
                    rlogger.debug('Sequence found via hint {hint_name} at index {i}'.format(
                        hint_name=hint.name, i=hint_result))
                    
                    # what we're getting back as hint_result is the signature
                    # "start_idx" - the index including all of the characters
                    # of the start of the iteration string.
                    # What we need to return is the "digit_idx", the index
                    # not including the previous digits (at least I think
                    # that's what digit_idx is.. wtf, i wrote this implementation
                    # and i don't remember what digit_idx really is)
                    
                    digit_idx = None
                    for entry in item.signature:
                        if entry['start_idx'] == hint_result:
                            digit_idx = entry['digit_idx']
                            break
                            
                    assert(digit_idx is not None)
                    
                    item_iter_result = _get_iter_entry(item, digit_idx)
                    
                    # got the iteration index!
                    self.__iter_idx = digit_idx
                    
                
            result = True
            
            
        elif item.signature == self[0].signature:
            
            iter_idx = _get_iter_index(self[0], item)
            
            if self.__iter_idx is None and iter_idx >= 0:
                
                '''
                rlogger.debug('setting iteration index: {value}'.format(
                    value=iter_idx))
                '''
                
                # the first entry if the sorted iteration list will be None,
                # since we didn't have an iteration index at the time of
                # insertion. Set the iteration entry for the first item
                first_item_iter_result = _get_iter_entry(self[0], iter_idx)
                
                # haha.. weird
                assert(self.__iter_digit_map[None] is not None)
                
                # reset the first item, which would've had None as the
                # iteration entry
                self.__iter_digit_map[first_item_iter_result] = self.__iter_digit_map[None]
                del self.__iter_digit_map[None]
                self.__sorted_iter_list.remove(None)
                self.__sorted_iter_list.append(first_item_iter_result)
                
                # set iteration index
                self.__iter_idx = iter_idx
                
                # return iteration entry result for this item
                item_iter_result = _get_iter_entry(item, iter_idx)
                result = True
                
            elif iter_idx == self.__iter_idx:
                
                item_iter_result = _get_iter_entry(item, iter_idx)
                
                result = True
                
        
        return (result, item_iter_result)
        
        
    def append(self, item):
        
        if type(item) is not Item:
            item = Item(item)
            
        (result, iter_entry) = self.match(item)
        if result:
            
            if iter_entry in self.__iter_digit_map:
                raise SequenceError('item {item} already part of sequence'.format(
                    item=item))
                
            self.__iter_digit_map[iter_entry] = item
            
            self._reset()
            
            
            item.sequence = self
            
            
        else:
            raise SequenceError('item {item} not part of sequence'.format(
                item=item))
                
