package com.pilacis;

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
            marked = false;
            lock = new ReentrantLock();
            nextNode = null;
        }

        T value;
        ReentrantLock lock;
        Node<T> nextNode;
        boolean marked;

        boolean isNotLast() {
            return this.nextNode.nextNode != null;
        }

        int compareTo(Node<T> node){
            if ((this.value == null && this.nextNode != null) || (node.value == null && node.nextNode == null)){
                return -1;
            }
            else if ((this.value == null && this.nextNode == null) || (node.value == null && node.nextNode != null)){
                return 1;
            }
            {
                return this.value.compareTo(node.value);
            }

        }
    }

    SafeSet() {
        head = new Node<>(null);
        head.nextNode = new Node<>(null);
    }

    Node<T> head;

    private Node<T> find(T value){
        Node<T> previous = head;
        Node<T> current = head.nextNode;
        while (previous.isNotLast() && current.value.compareTo(value) < 0) {
            previous = current;
            current = current.nextNode;
        }
        return previous;
    }

    @Override
    public boolean add(T value) {
        do {
            Node<T> previous = find(value);
            Node<T> current = previous.nextNode;

            previous.lock.lock();
            current.lock.lock();

            if (validate(previous, current)) {
                if (previous.isNotLast() && current.value.compareTo(value) == 0) {
                    previous.lock.unlock();
                    current.lock.unlock();
                    return false;
                } else {
                    Node<T> newNode = new Node<>(value);
                    newNode.nextNode = current;
                    previous.nextNode = newNode;
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
            Node<T> current = previous.nextNode;

            if (validate(previous, current)) {
                if (previous.isNotLast() && current.value.compareTo(value) == 0) {
                    current.marked = true;
                    previous.nextNode = current.nextNode;
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
        Node<T> current = previous.nextNode;
        return current.value.compareTo(value) == 0 && !current.marked;
    }

    public boolean validate(Node<T> previous, Node<T> current) {
        return (previous.nextNode == current && !previous.marked && !current.marked);
    }


    @Override
    public boolean isEmpty() {
        return head.nextNode.value == null;
    }

    public String getElements() {
        StringBuilder str = new StringBuilder();
        Node<T> current = head.nextNode;
        while (current.value != null) {
            str.append(current.value.toString()).append(" ");
            current = current.nextNode;
        }
        return str.toString();
    }
}