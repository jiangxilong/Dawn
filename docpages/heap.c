/**
 * @page Heaps Heaps and dynamic memory allocatiopn
 * @section TheHeap Heap
 * Heaps are of critical importance to a operating system. They allow the dynamic allocation of data. This meens that they allow the kernel to keep track of what memory is in use and what is not on a much more precise level then the frame and page allocaters (Which can keep track of what memory “pages” are in use and what are not. Pages are typically 4kb (On x86 systems) in size). Without one the kernel and therefore the operating system will be very limited in functionality. The drawbacks of heaps is that they have a memory offset, in the implementation that I plan to use 16 bytes of memory are required for heap ‘headers’, these headers however are essential and this overhead is minuscule.
 *
 */
