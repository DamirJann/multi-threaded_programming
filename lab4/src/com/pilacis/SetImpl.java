package com.pilacis;

import java.util.concurrent.atomic.AtomicMarkableReference;


class SetImpl<T extends Comparable<T>> implements Set<T> {


    static class Node<T extends Comparable<T>> {
        Node(T value) {
            this.value = value;
            next = new AtomicMarkableReference<>(null, false);
        }

        T value;
        AtomicMarkableReference<Node<T>> next;

    }

    SetImpl() {
        head = new Node<>(null);
        tail = new Node<>(null);
        head.next = new AtomicMarkableReference<>(tail, false);
    }

    Node<T> head;
    Node<T> tail;


    private Node<T> find(T value) {
        Node<T> previous = head;
        Node<T> current = previous.next.getReference();
        Node<T> afterCurrent = current.next.getReference();

        while (true) {
            if (current.next.isMarked()) {
                // cas not passed -> start search with beginning
                // the case when before cas it is added new element after previous
                if (!previous.next.compareAndSet(current, afterCurrent, false, false)) {
                    previous = head;
                    current = previous.next.getReference();
                    afterCurrent = current.next.getReference();
                }
            } else {
                if (current != tail && current.value.compareTo(value) < 0) {
                    previous = current;
                    current = afterCurrent;
                    afterCurrent = afterCurrent.next.getReference();
                } else {
                    return previous;
                }
            }
        }
    }

    @Override
    public boolean add(T value) {
        do {
            Node<T> previous = find(value);
            Node<T> current = previous.next.getReference();

            if (current != tail && current.value.compareTo(value) == 0) {
                return false;
            } else {
                Node<T> newNode = new Node<>(value);
                newNode.next.set(current, false);
                // cas will not work when:
                // 1. previous node is deleted -> Mark = false
                // 2. added new element before: previous -> new elem -> current
                if (previous.next.compareAndSet(current, newNode, false, false)) {
                    return true;
                }
            }
        } while (true);


    }

    @Override
    public boolean remove(T value) {
        do {
            Node<T> previous = find(value);
            Node<T> current = previous.next.getReference();

            if (current != tail && current.value.compareTo(value) == 0) {
                Node<T> nextCurrent = current.next.getReference();
                if (current.next.compareAndSet(nextCurrent, nextCurrent, false, true)) {
                    return true;
                }
            } else {
                return false;
            }
        } while (true);

    }


    @Override
    public boolean contains(T value) {
        Node<T> current = head.next.getReference();
        while (current != tail && current.value.compareTo(value) < 0) {
            current = current.next.getReference();
        }
        return current != tail && !current.next.isMarked() && current.value.compareTo(value) == 0;
    }


    @Override
    public boolean isEmpty() {

        while (head.next.getReference() != tail) {
            Node<T> previous = head,
                    current = previous.next.getReference(), // logically deleted
                    afterCurrent = current.next.getReference();

            if (current.next.isMarked()) {
                previous.next.compareAndSet(current, afterCurrent, false, false);
            } else {
                return false;
            }
        }
        return true;
    }

    public String getElements() {
        StringBuilder str = new StringBuilder();
        Node<T> current = head.next.getReference();
        while (current.value != null && !current.next.isMarked()) {
            str.append(current.value.toString()).append(" ");
            current = current.next.getReference();
        }
        return str.toString();
    }
}

