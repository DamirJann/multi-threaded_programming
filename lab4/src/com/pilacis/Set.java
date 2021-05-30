package com.pilacis;

import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Lock-Free множество.
 *
 * @param <T> тип ключей
 */
public interface Set<T extends Comparable<T>> {
    /**
     * Добавить ключ к множеству
     * <p>
     * Алгоритм должен быть как минимум lock-free
     *
     * @param value значение ключа
     * @return false если value уже существует в множестве,
     * true если элемент был добавлен
     */
    boolean add(T value);

    /**
     * Удалить ключ из множества
     * <p>
     * Алгоритм должен быть как минимум lock-free
     *
     * @param value значение ключа
     * @return false если ключ не был найден, true если ключ успешно удален
     */
    boolean remove(T value);

    /**
     * Проверка наличия ключа в множестве
     * <p>
     * Алгоритм должен быть как минимум wait-free
     *
     * @param value значение ключа
     * @return true если элемент содержится в множестве, иначе - false
     */
    boolean contains(T value);

    /**
     * Проверка множества на пустоту
     * <p>
     * Алгоритм должен быть как минимум lock-free
     *
     * @return true если множество пусто, иначе - false
     */
    boolean isEmpty();
}


class SafeSet<T extends Comparable<T>> implements Set<T> {


    static class Node<T extends Comparable<T>> {
        Node(T value) {
            this.value = value;
            lock = new ReentrantLock();
            nextAndMarked = new AtomicReference<>();
            nextAndMarked.set(new NextAndMarked(null, false));
        }

        private static class NextAndMarked<T extends Comparable<T>>{
            NextAndMarked(Node<T> next, boolean marked){
                this.next = next;
                this.marked = marked;
            }
            Node<T> next;
            boolean marked;
        }

        void setMarked(boolean marked){
            nextAndMarked.set(new NextAndMarked(nextAndMarked.get().next, marked));
        }
        boolean getMarked(){
            return nextAndMarked.get().marked;
        }
        void setNext(Node<T> next){
            nextAndMarked.set(new NextAndMarked(next, nextAndMarked.get().marked));
        }
        Node<T> getNext(){
            return nextAndMarked.get().next;
        }

        T value;
        ReentrantLock lock;
        AtomicReference<NextAndMarked> nextAndMarked;

        boolean isNotLast() {
            return this.getNext().getNext() != null;
        }

    }

    SafeSet() {
        head = new Node<>(null);
        head.setNext(new Node<>(null));
    }

    Node<T> head;

    private Node<T> find(T value) {
        Node<T> previous = head;
        Node<T> current = head.getNext();
        while (previous.isNotLast() && current.value.compareTo(value) < 0) {
            previous = current;
            current = current.getNext();
        }
        return previous;
    }

    @Override
    public boolean add(T value) {
        do {
            Node<T> previous = find(value);
            Node<T> current = previous.getNext();

            previous.lock.lock();
            current.lock.lock();

            if (validate(previous, current)) {
                if (previous.isNotLast() && current.value.compareTo(value) == 0) {
                    previous.lock.unlock();
                    current.lock.unlock();
                    return false;
                } else {
                    Node<T> newNode = new Node<>(value);
                    newNode.setNext(current);
                    previous.setNext(newNode);
                    previous.lock.unlock();
                    current.lock.unlock();
                    return true;
                }
            } else {
                current.lock.unlock();
                previous.lock.unlock();
            }

        } while (true);


    }

    @Override
    public boolean remove(T value) {
        do {
            Node<T> previous = find(value);
            Node<T> current = previous.getNext();

            if (validate(previous, current)) {
                if (previous.isNotLast() && current.value.compareTo(value) == 0) {
                    current.setMarked(true);
                    previous.setNext(current.getNext());
                    previous.nextAndMarked.compareAndSet(new Node.NextAndMarked<>(current, false), new Node.NextAndMarked<>(current.getNext(), false));
                    return true;
                } else {
                    return false;
                }
            } else {
                current.lock.unlock();
                previous.lock.unlock();
            }

        } while (true);

    }


    @Override
    public boolean contains(T value) {
        Node<T> previous = find(value);
        Node<T> current = previous.getNext();
        return previous.isNotLast() && current.value.compareTo(value) == 0 && !current.getMarked();
    }

    public boolean validate(Node<T> previous, Node<T> current) {
        return (previous.getNext() == current && !previous.getMarked() && !current.getMarked());
    }


    @Override
    public boolean isEmpty() {
        return head.getNext().value == null;
    }

    public String getElements() {
        StringBuilder str = new StringBuilder();
        Node<T> current = head.getNext();
        while (current.value != null) {
            str.append(current.value.toString()).append(" ");
            current = current.getNext();
        }
        return str.toString();
    }
}