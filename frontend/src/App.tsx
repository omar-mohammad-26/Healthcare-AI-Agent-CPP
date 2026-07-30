import { useMemo, useState, type FormEventHandler } from 'react'
import './App.css'

type MessageRole = 'assistant' | 'user'

type AgentStep = {
  number: number
  name: string
}

type ChatMessage = {
  id: number
  role: MessageRole
  content: string
  steps?: AgentStep[]
}

type AgentResponse = {
  status: string
  answer: string
  step_count: number
  steps: AgentStep[]
}

const examplePrompts = [
  'Find the patient Sarah Cohen.',
  'List all doctors.',
  'Show available slots for doctor Amit Patel on 2026-08-03.',
]

function App() {
  const [messages, setMessages] = useState<ChatMessage[]>([
    {
      id: 1,
      role: 'assistant',
      content:
          'Hello! I am the Healthcare AI Agent. I can search patients, list doctors, check schedules, find available slots, book or cancel appointments, and review treatment history.',
    },
  ])

  const [prompt, setPrompt] = useState('')
  const [isSending, setIsSending] = useState(false)

  const nextMessageId = useMemo(
      () =>
          messages.reduce(
              (maximum, message) => Math.max(maximum, message.id),
              0,
          ) + 1,
      [messages],
  )

  const sendPrompt: FormEventHandler<HTMLFormElement> = async (event) => {
    event.preventDefault()

    const trimmedPrompt = prompt.trim()

    if (!trimmedPrompt || isSending) {
      return
    }

    const userMessage: ChatMessage = {
      id: nextMessageId,
      role: 'user',
      content: trimmedPrompt,
    }

    setMessages((currentMessages) => [
      ...currentMessages,
      userMessage,
    ])

    setPrompt('')
    setIsSending(true)

    try {
      const response = await fetch('http://localhost:8000/agent/run', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          prompt: trimmedPrompt,
        }),
      })

      const responseBody = await response.json()

      if (!response.ok) {
        const errorMessage =
            responseBody.message ??
            'The Healthcare AI Agent request failed.'

        setMessages((currentMessages) => [
          ...currentMessages,
          {
            id: nextMessageId + 1,
            role: 'assistant',
            content: `Error: ${errorMessage}`,
          },
        ])

        return
      }

      const agentResponse = responseBody as AgentResponse

      const assistantMessage: ChatMessage = {
        id: nextMessageId + 1,
        role: 'assistant',
        content: agentResponse.answer,
        steps: agentResponse.steps,
      }

      setMessages((currentMessages) => [
        ...currentMessages,
        assistantMessage,
      ])
    } catch (error) {
      const errorMessage =
          error instanceof Error
              ? error.message
              : 'Unable to connect to the Healthcare AI Agent.'

      setMessages((currentMessages) => [
        ...currentMessages,
        {
          id: nextMessageId + 1,
          role: 'assistant',
          content: `Error: ${errorMessage}`,
        },
      ])
    } finally {
      setIsSending(false)
    }
  }

  const chooseExamplePrompt = (examplePrompt: string) => {
    setPrompt(examplePrompt)
  }

  return (
      <main className="min-vh-100 bg-body-tertiary">
        <nav className="navbar navbar-expand-lg bg-white border-bottom shadow-sm">
          <div className="container py-2">
          <span className="navbar-brand d-flex align-items-center gap-3 mb-0">
            <span className="brand-icon d-inline-flex align-items-center justify-content-center">
              <i
                  className="bi bi-heart-pulse-fill"
                  aria-hidden="true"
              />
            </span>

            <span>
              <span className="d-block fw-bold">
                Healthcare AI Agent
              </span>

              <small className="d-block text-body-secondary fw-normal">
                Intelligent clinic management assistant
              </small>
            </span>
          </span>

            <span className="badge rounded-pill text-bg-success">
            <i
                className="bi bi-circle-fill me-2 status-dot"
                aria-hidden="true"
            />
            Backend ready
          </span>
          </div>
        </nav>

        <section className="container py-4 py-lg-5">
          <div className="row justify-content-center">
            <div className="col-12 col-xl-10">
              <div className="row g-4">
                <aside className="col-lg-4">
                  <div className="card border-0 shadow-sm h-100">
                    <div className="card-body p-4">
                      <h1 className="h4 fw-bold mb-2">
                        Clinic Assistant
                      </h1>

                      <p className="text-body-secondary mb-4">
                        Ask questions using natural language. The Agent
                        selects and executes the required healthcare tools
                        automatically.
                      </p>

                      <h2 className="h6 text-uppercase text-body-secondary fw-bold mb-3">
                        Example questions
                      </h2>

                      <div className="d-grid gap-2">
                        {examplePrompts.map((examplePrompt) => (
                            <button
                                key={examplePrompt}
                                type="button"
                                className="btn btn-light border text-start p-3"
                                onClick={() =>
                                    chooseExamplePrompt(examplePrompt)
                                }
                            >
                              <i
                                  className="bi bi-chat-left-text me-2 text-primary"
                                  aria-hidden="true"
                              />
                              {examplePrompt}
                            </button>
                        ))}
                      </div>

                      <hr className="my-4" />

                      <div className="small text-body-secondary">
                        <div className="d-flex align-items-center gap-2 mb-2">
                          <i
                              className="bi bi-shield-check text-success"
                              aria-hidden="true"
                          />
                          Fictional demonstration data
                        </div>

                        <div className="d-flex align-items-center gap-2">
                          <i
                              className="bi bi-tools text-primary"
                              aria-hidden="true"
                          />
                          Eight registered Agent tools
                        </div>
                      </div>
                    </div>
                  </div>
                </aside>

                <div className="col-lg-8">
                  <div className="card border-0 shadow-sm chat-card">
                    <div className="card-header bg-white border-bottom p-3 p-md-4">
                      <div className="d-flex align-items-center gap-3">
                      <span className="agent-avatar d-inline-flex align-items-center justify-content-center">
                        <i
                            className="bi bi-robot"
                            aria-hidden="true"
                        />
                      </span>

                        <div>
                          <h2 className="h6 fw-bold mb-1">
                            Healthcare Agent Chat
                          </h2>

                          <p className="small text-body-secondary mb-0">
                            Powered by Gemini and C++ Function Tools
                          </p>
                        </div>
                      </div>
                    </div>

                    <div
                        className="card-body chat-messages p-3 p-md-4"
                        aria-live="polite"
                    >
                      {messages.map((message) => (
                          <article
                              key={message.id}
                              className={`message-row ${
                                  message.role === 'user'
                                      ? 'message-row-user'
                                      : 'message-row-assistant'
                              }`}
                          >
                            <div
                                className={`message-bubble ${
                                    message.role === 'user'
                                        ? 'message-bubble-user'
                                        : 'message-bubble-assistant'
                                }`}
                            >
                              <div className="message-content">
                                {message.content}
                              </div>

                              {message.steps &&
                                  message.steps.length > 0 && (
                                      <div className="agent-steps mt-3 pt-3 border-top">
                                        <div className="small fw-bold mb-2">
                                          Agent tools used
                                        </div>

                                        <div className="d-flex flex-wrap gap-2">
                                          {message.steps.map((step) => (
                                              <span
                                                  key={`${message.id}-${step.number}`}
                                                  className="badge text-bg-light border text-body"
                                              >
                                      {step.number}. {step.name}
                                    </span>
                                          ))}
                                        </div>
                                      </div>
                                  )}
                            </div>
                          </article>
                      ))}

                      {isSending && (
                          <div className="message-row message-row-assistant">
                            <div className="message-bubble message-bubble-assistant">
                          <span
                              className="spinner-border spinner-border-sm me-2"
                              aria-hidden="true"
                          />
                              The Agent is selecting and executing tools...
                            </div>
                          </div>
                      )}
                    </div>

                    <div className="card-footer bg-white border-top p-3 p-md-4">
                      <form onSubmit={sendPrompt}>
                        <label
                            htmlFor="agent-prompt"
                            className="visually-hidden"
                        >
                          Ask the Healthcare AI Agent
                        </label>

                        <div className="input-group input-group-lg">
                          <input
                              id="agent-prompt"
                              type="text"
                              className="form-control"
                              placeholder="Ask about patients, doctors, appointments..."
                              value={prompt}
                              onChange={(event) =>
                                  setPrompt(event.target.value)
                              }
                              disabled={isSending}
                              autoComplete="off"
                          />

                          <button
                              type="submit"
                              className="btn btn-primary px-4"
                              disabled={!prompt.trim() || isSending}
                          >
                            <i
                                className="bi bi-send-fill me-2"
                                aria-hidden="true"
                            />
                            Send
                          </button>
                        </div>
                      </form>

                      <p className="small text-body-secondary text-center mt-3 mb-0">
                        This project uses fictional healthcare information
                        for demonstration purposes only.
                      </p>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </section>
      </main>
  )
}

export default App