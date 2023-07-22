package main.assembly;

public abstract class MemoryReference extends Operand implements Comparable<MemoryReference> {
    public boolean isMemoryReference() {
        return true;
    }

    protected abstract  void fixOffset(long diff);
   protected  abstract  int cmp(DirectMemoryReference mem);
   protected abstract  int cmp(IndirectMemoryReference mem);
}
