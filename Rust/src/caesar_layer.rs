use std::sync::Arc;
use opendal::raw::*;
use opendal::*;

pub struct CaesarCipherLayer {
    shift: u8,
}

impl CaesarCipherLayer {
    pub fn new(shift: u8) -> Self {
        Self { shift }
    }
    
    fn encrypt_buffer(&self, buffer: Buffer) -> Buffer {
        let bytes = buffer.to_bytes();
        let encrypted = bytes
            .iter()
            .map(|&b| {
                if b.is_ascii_alphabetic() {
                    let base = if b.is_ascii_lowercase() { b'a' } else { b'A' };
                    let offset = (b - base + self.shift) % 26;
                    base + offset
                } else {
                    b
                }
            })
            .collect::<Vec<u8>>();
        
        Buffer::from(encrypted)
    }
    
    fn decrypt_buffer(&self, buffer: Buffer) -> Buffer {
        let bytes = buffer.to_bytes();
        let decrypted = bytes
            .iter()
            .map(|&b| {
                if b.is_ascii_alphabetic() {
                    let base = if b.is_ascii_lowercase() { b'a' } else { b'A' };
                    let offset = (b - base + (26 - (self.shift % 26))) % 26;
                    base + offset
                } else {
                    b
                }
            })
            .collect::<Vec<u8>>();
        
        Buffer::from(decrypted)
    }
}

impl<A: Access> Layer<A> for CaesarCipherLayer {
    type LayeredAccess = CaesarCipherAccessor<A>;

    fn layer(&self, inner: A) -> Self::LayeredAccess {
        CaesarCipherAccessor { 
            inner,
            shift: self.shift,
        }
    }
}

#[derive(Debug)]
pub struct CaesarCipherAccessor<A> {
    inner: A,
    shift: u8,
}

pub struct CaesarCipherReader<R> {
    inner: R,
    shift: u8,
}

impl<R: oio::Read> oio::Read for CaesarCipherReader<R> {
    async fn read(&mut self) -> Result<Buffer> {
        let buffer = self.inner.read().await?;
        let bytes = buffer.to_bytes();

        println!("CaesarCipherReader (Received): {:?}", bytes.to_vec());

        let decrypted = bytes
            .iter()
            .map(|&b| {
                if b.is_ascii_alphabetic() {
                    let base = if b.is_ascii_lowercase() { b'a' } else { b'A' };
                    let offset = (b - base + (26 - (self.shift % 26))) % 26;
                    base + offset
                } else {
                    b
                }
            })
            .collect::<Vec<u8>>();
        
        println!("CaesarCipherReader (Sent): {:?}", decrypted);
        Ok(Buffer::from(decrypted))
    }
}

pub struct CaesarCipherWriter<W> {
    inner: W,
    shift: u8,
}

impl<W: oio::Write> oio::Write for CaesarCipherWriter<W> {
    fn write(&mut self, bs: Buffer) -> impl std::future::Future<Output = Result<()>> + std::marker::Send {
        let bytes = bs.to_bytes();

        println!("CaesarCipherWriter (Received): {:?}", bytes.to_vec());

        let encrypted = bytes
            .iter()
            .map(|&b| {
                if b.is_ascii_alphabetic() {
                    let base = if b.is_ascii_lowercase() { b'a' } else { b'A' };
                    let offset = (b - base + self.shift) % 26;
                    base + offset
                } else {
                    b
                }
            })
            .collect::<Vec<u8>>();
        
        println!("CaesarCipherWriter (Sent): {:?}", encrypted);
        self.inner.write(Buffer::from(encrypted))
    }

    fn abort(&mut self) -> impl std::future::Future<Output = Result<()>> + std::marker::Send {
        self.inner.abort()
    }

    fn close(&mut self) -> impl std::future::Future<Output = Result<Metadata>> + std::marker::Send {
        self.inner.close()
    }
}

impl<A: Access> LayeredAccess for CaesarCipherAccessor<A> {
    type Inner = A;
    type Reader = CaesarCipherReader<A::Reader>;
    type BlockingReader = A::BlockingReader;
    type Writer = CaesarCipherWriter<A::Writer>;
    type BlockingWriter = A::BlockingWriter;
    type Lister = A::Lister;
    type BlockingLister = A::BlockingLister;
    type Deleter = A::Deleter;
    type BlockingDeleter = A::BlockingDeleter;

    fn inner(&self) -> &Self::Inner {
        &self.inner
    }

    fn info(&self) -> Arc<AccessorInfo> {
        self.inner.info()
    }

    async fn read(&self, path: &str, args: OpRead) -> Result<(RpRead, Self::Reader)> {
        let (rp, reader) = self.inner.read(path, args).await?;
        Ok((rp, CaesarCipherReader { inner: reader, shift: self.shift }))
    }

    async fn write(&self, path: &str, args: OpWrite) -> Result<(RpWrite, Self::Writer)> {
        let (rp, writer) = self.inner.write(path, args).await?;
        Ok((rp, CaesarCipherWriter { inner: writer, shift: self.shift }))
    }

    async fn create_dir(&self, path: &str, args: OpCreateDir) -> Result<RpCreateDir> {
        self.inner.create_dir(path, args).await
    }

    async fn copy(&self, from: &str, to: &str, args: OpCopy) -> Result<RpCopy> {
        self.inner.copy(from, to, args).await
    }

    async fn rename(&self, from: &str, to: &str, args: OpRename) -> Result<RpRename> {
        self.inner.rename(from, to, args).await
    }

    async fn stat(&self, path: &str, args: OpStat) -> Result<RpStat> {
        self.inner.stat(path, args).await
    }

    async fn delete(&self) -> Result<(RpDelete, Self::Deleter)> {
        self.inner.delete().await
    }

    async fn list(&self, path: &str, args: OpList) -> Result<(RpList, Self::Lister)> {
        self.inner.list(path, args).await
    }

    async fn presign(&self, path: &str, args: OpPresign) -> Result<RpPresign> {
        self.inner.presign(path, args).await
    }

    fn blocking_create_dir(&self, path: &str, args: OpCreateDir) -> Result<RpCreateDir> {
        self.inner.blocking_create_dir(path, args)
    }

    fn blocking_read(&self, path: &str, args: OpRead) -> Result<(RpRead, Self::BlockingReader)> {
        self.inner.blocking_read(path, args)
    }

    fn blocking_write(&self, path: &str, args: OpWrite) -> Result<(RpWrite, Self::BlockingWriter)> {
        self.inner.blocking_write(path, args)
    }

    fn blocking_copy(&self, from: &str, to: &str, args: OpCopy) -> Result<RpCopy> {
        self.inner.blocking_copy(from, to, args)
    }

    fn blocking_rename(&self, from: &str, to: &str, args: OpRename) -> Result<RpRename> {
        self.inner.blocking_rename(from, to, args)
    }

    fn blocking_stat(&self, path: &str, args: OpStat) -> Result<RpStat> {
        self.inner.blocking_stat(path, args)
    }

    fn blocking_delete(&self) -> Result<(RpDelete, Self::BlockingDeleter)> {
        self.inner.blocking_delete()
    }

    fn blocking_list(&self, path: &str, args: OpList) -> Result<(RpList, Self::BlockingLister)> {
        self.inner.blocking_list(path, args)
    }
}


/*
Assuming read operation:

- op.read(stuff)                        OPERATOR CALL
- logging_layer                         LAYER "PIPELINE" CALL
----------
- caesar_layer                          LAYER "PIPELINE" CALL
  - CaesarCipherAccessor                ENTRY POINT
  - read                                CALLS INNER IMPLEMENTATION
    - CaesarCipherReader                INNER IMPLEMENTATION
    - read (encryption happens)         REAL PROCESSING
----------
- fastrace_layer                        LAYER "PIPELINE" CALL


Output:

CaesarCipherWriter (Received): [72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33]
CaesarCipherWriter (Sent):     [75, 104, 111, 111, 114, 44, 32, 90, 114, 117, 111, 103, 33]

CaesarCipherReader (Received): [75, 104, 111, 111, 114, 44, 32, 90, 114, 117, 111, 103, 33]
CaesarCipherReader (Sent):     [72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33]
*/
